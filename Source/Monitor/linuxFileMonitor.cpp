#include "Monitor/linuxFileMonitor.h"

#ifdef Q_OS_LINUX

#include "constants.h"

#include <sstream>
#include <system_error>

#include <unistd.h>

#include <gsl/gsl_assert>
#include <spdlog/spdlog.h>

LinuxFileMonitor::~LinuxFileMonitor() noexcept
{
    if (m_isActive.load()) {
        Stop();
    }

    // @todo Might not want to call this unconditionally.
    CleanUpInotify();
}

void LinuxFileMonitor::Start(
    const std::filesystem::path& path, std::function<void(FileEvent&&)> onNotificationCallback)
{
    m_pathToWatch = path;
    m_notificationCallback = std::move(onNotificationCallback);

    InitializeInotify();
    RegisterWatchersRecursively(path);

    m_isActive.store(true);
    m_monitoringThread = std::thread{ [&] { Monitor(); } };
}

void LinuxFileMonitor::InitializeInotify()
{
    m_inotifyFileDescriptor = ::inotify_init1(IN_NONBLOCK);
    if (m_inotifyFileDescriptor == -1) {
        const std::string lastError = ::strerror(errno);
        throw std::runtime_error("Couldn't initialize inotify " + lastError + ".");
    }

    m_epollFileDescriptor = ::epoll_create1(0);
    if (m_epollFileDescriptor == -1) {
        const std::string lastError = ::strerror(errno);
        throw std::runtime_error("Couldn't initialize epoll " + lastError + ".");
    }

    m_inotifyEpollEvent.events = EPOLLIN | EPOLLET;
    m_inotifyEpollEvent.data.fd = m_inotifyFileDescriptor;

    auto epollResult = ::epoll_ctl(
        m_epollFileDescriptor, EPOLL_CTL_ADD, m_inotifyFileDescriptor, &m_inotifyEpollEvent);

    if (epollResult == -1) {
        const std::string lastError = ::strerror(errno);
        throw std::runtime_error(
            "Couldn't add inotify file descriptor to epoll. Error: " + lastError + ".");
    }

    m_stopEventFileDescriptor = ::eventfd(0, EFD_NONBLOCK);
    m_stopEpollEvent.data.fd = m_stopEventFileDescriptor;
    m_stopEpollEvent.events = EPOLLIN | EPOLLET;

    epollResult = ::epoll_ctl(
        m_epollFileDescriptor, EPOLL_CTL_ADD, m_stopEventFileDescriptor, &m_stopEpollEvent);

    if (epollResult == -1) {
        const std::string lastError = ::strerror(errno);
        throw std::runtime_error("Couldn't add stop event to epoll. Error: " + lastError + ".");
    }
}

void LinuxFileMonitor::CleanUpInotify() noexcept
{
    ::epoll_ctl(m_epollFileDescriptor, EPOLL_CTL_DEL, m_inotifyFileDescriptor, nullptr);

    if (!::close(m_inotifyFileDescriptor)) {
        const auto lastError = errno;
        const auto& log = spdlog::get(Constants::Logging::DefaultLog);
        log->error("Encountered an error closing inotify file descriptor. Error: {}.", lastError);
    }

    if (!::close(m_epollFileDescriptor)) {
        const auto lastError = errno;
        const auto& log = spdlog::get(Constants::Logging::DefaultLog);
        log->error("Encountered an error closing epoll file descriptor. Error: {}.", lastError);
    }
}

void LinuxFileMonitor::RegisterWatchersRecursively(const std::filesystem::path& path)
{
    std::vector<std::filesystem::path> paths;

    if (!std::filesystem::exists(path)) {
        throw std::invalid_argument(
            "Cannot watch a path that does not exist. Path: " + path.string());
    }

    if (std::filesystem::is_directory(path)) {
        std::error_code errorCode;

        std::filesystem::recursive_directory_iterator itr{
            path, std::filesystem::directory_options::follow_directory_symlink, errorCode
        };

        std::filesystem::recursive_directory_iterator end;

        for (; itr != end; itr.increment(errorCode)) {
            const std::filesystem::path& currentPath = *itr;

            if (!std::filesystem::is_directory(currentPath) &&
                !std::filesystem::is_symlink(currentPath)) {
                continue;
            }

            paths.push_back(currentPath);
        }
    }

    paths.push_back(path);

    for (auto& path : paths) {
        RegisterWatcher(path);
    }
}

void LinuxFileMonitor::RegisterWatcher(const std::filesystem::path& path)
{
    const auto absolutePath = std::filesystem::absolute(path);

    constexpr auto flags = IN_MODIFY | IN_IGNORED | IN_DELETE | IN_DELETE_SELF;

    const int watchDescriptor =
        ::inotify_add_watch(m_inotifyFileDescriptor, absolutePath.string().c_str(), flags);

    if (watchDescriptor != -1) {
        m_watchDescriptorToPathMap.emplace(watchDescriptor, path);
    }

    const auto lastError = errno;
    const std::string errorMessage = ::strerror(lastError);

    switch (lastError) {
        case 2: {
            return;
        }
        case 13: {
            const auto& log = spdlog::get(Constants::Logging::DefaultLog);
            log->error("Denied permission to set watch on: {}.", absolutePath.string());

            return;
        }
        case 28: {
            throw std::runtime_error(
                "Exceeded watch limit. Edit \"/proc/sys/fs/inotify/max_user_watches\" to increase "
                "limit.");
        }
        default: {
            throw std::runtime_error("Failed to register watch. Error: " + errorMessage + ".");
        }
    }
}

void LinuxFileMonitor::AwaitNotification()
{
    constexpr auto timeout = -1;
    const auto eventsRead =
        ::epoll_wait(m_epollFileDescriptor, m_epollEvents, m_maxEpollEvents, timeout);

    if (eventsRead == -1) {
        return;
    }

    ssize_t bytesRead = 0;

    for (auto i = 0; i < eventsRead; ++i) {
        if (m_epollEvents[i].data.fd == m_stopEventFileDescriptor) {
            break;
        }

        bytesRead = ::read(m_epollEvents[i].data.fd, m_eventBuffer.data(), m_eventBuffer.size());

        if (bytesRead == -1) {
            const auto lastError = errno;
            if (lastError == EINTR) {
                const auto& log = spdlog::get(Constants::Logging::DefaultLog);
                log->error("Encountered an error reading epoll events. Error: {}.", lastError);

                break;
            }
        }

        ProcessEvents(bytesRead);
    }
}

void LinuxFileMonitor::ProcessEvents(long bytesAvailable)
{
    auto offset = 0;

    while (offset < bytesAvailable) {
        auto* const event = reinterpret_cast<inotify_event*>(m_eventBuffer.data() + offset);

        if (event->mask & IN_IGNORED) {
            offset += m_eventSize + event->len;
            m_watchDescriptorToPathMap.erase(event->wd);

            continue;
        }

        const auto itr = m_watchDescriptorToPathMap.find(event->wd);
        if (itr == std::end(m_watchDescriptorToPathMap)) {
            const auto& log = spdlog::get(Constants::Logging::DefaultLog);
            log->error("Encountered an error associating epoll event with corresponding file.");
        }

        const auto path = itr->second / std::filesystem::path{ event->name };

        switch (event->mask) {
            case IN_MODIFY:
                m_notificationCallback(FileEvent{ path, FileEventType::TOUCHED });
                break;
            case IN_DELETE_SELF:
                [[fallthrough]];
            case IN_DELETE:
                m_notificationCallback(FileEvent{ path, FileEventType::DELETED });
                break;
        }

        offset += m_eventSize + event->len;
    }
}

void LinuxFileMonitor::Monitor()
{
    while (m_keepMonitoring) {
        AwaitNotification();
    }

    m_isActive.store(false);
}

void LinuxFileMonitor::Stop()
{
    if (m_isActive.load() == false) {
        return;
    }

    m_keepMonitoring.store(false);

    ::eventfd_write(m_stopEventFileDescriptor, 1);

    if (m_monitoringThread.joinable()) {
        m_monitoringThread.join();
    }

    Expects(m_isActive.load() == false);
}

bool LinuxFileMonitor::IsActive() const
{
    return m_isActive.load();
}

#endif // Q_OS_LINUX
