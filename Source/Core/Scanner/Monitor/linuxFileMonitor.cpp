#include "Scanner/Monitor/linuxFileMonitor.h"

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
    if (pipe2(m_stopPipeFileDescriptor, O_NONBLOCK) == -1) {
        const std::string lastError = strerror(errno);
        const auto message = "Can't initialize stop pipe! " + lastError + ".";
        throw std::runtime_error(message);
    }

    m_inotifyFileDescriptor = inotify_init1(IN_NONBLOCK);
    if (m_inotifyFileDescriptor == -1) {
        const std::string lastError = strerror(errno);
        const auto message = "Can't initialize inotify " + lastError + ".";
        throw std::runtime_error(message);
    }

    m_epollFileDescriptor = epoll_create1(0);
    if (m_epollFileDescriptor == -1) {
        const std::string lastError = strerror(errno);
        const auto message = "Can't initialize epoll " + lastError + ".";
        throw std::runtime_error(message);
    }

    m_inotifyEpollEvent.events = EPOLLIN | EPOLLET;
    m_inotifyEpollEvent.data.fd = m_inotifyFileDescriptor;

    auto epollResult = epoll_ctl(
        m_epollFileDescriptor, EPOLL_CTL_ADD, m_inotifyFileDescriptor, &m_inotifyEpollEvent);

    if (epollResult == -1) {
        const std::string lastError = strerror(errno);
        const auto message = "Can't inotify file descriptor to epoll. Error: " + lastError + ".";
        throw std::runtime_error(message);
    }

    m_stopPipeEpollEvent.events = EPOLLIN | EPOLLET;
    m_stopPipeEpollEvent.data.fd = m_stopPipeFileDescriptor[m_pipeReadIndex];

    epollResult = epoll_ctl(
        m_epollFileDescriptor, EPOLL_CTL_ADD, m_stopPipeFileDescriptor[m_pipeReadIndex],
        &m_stopPipeEpollEvent);

    if (epollResult == -1) {
        const std::string lastError = strerror(errno);
        const auto message = "Can't add pipe filedescriptor to epoll!  Error: " + lastError + ".";
        throw std::runtime_error(message);
    }
}

void LinuxFileMonitor::CleanUpInotify() noexcept
{
    epoll_ctl(m_epollFileDescriptor, EPOLL_CTL_DEL, m_inotifyFileDescriptor, nullptr);
    epoll_ctl(
        m_epollFileDescriptor, EPOLL_CTL_DEL, m_stopPipeFileDescriptor[m_pipeReadIndex], nullptr);

    if (!close(m_inotifyFileDescriptor)) {
        // const auto lastError = errno;
        // @todo Log error...
    }

    if (!close(m_epollFileDescriptor)) {
        // const auto lastError = errno;
        // @todo Log error...
    }

    close(m_stopPipeFileDescriptor[m_pipeReadIndex]);
    close(m_stopPipeFileDescriptor[m_pipeWriteIndex]);
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
    const int watchDescriptor =
        inotify_add_watch(m_inotifyFileDescriptor, path.string().c_str(), IN_ALL_EVENTS);

    if (watchDescriptor == -1) {
        const auto lastError = errno;
        const std::string errorMessage = strerror(lastError);
        if (lastError == 28) {
            const auto message =
                "Exceeded watch limit. Edit \"/proc/sys/fs/inotify/max_user_watches\" to increase "
                "limit. Error: " +
                errorMessage + ".";

            throw std::runtime_error(message);
        }

        if (lastError == 2) {
            return;
        }

        const auto message = "Failed to register watch. Error: " + errorMessage + ".";
        throw std::runtime_error(message);
    }

    m_watchDescriptorToPathMap.emplace(watchDescriptor, path);
}

void LinuxFileMonitor::AwaitNotification()
{
    constexpr auto timeout = -1;
    const auto eventsRead =
        epoll_wait(m_epollFileDescriptor, m_epollEvents, m_maxEpollEvents, timeout);

    if (eventsRead == -1) {
        return;
    }

    long bytesRead = 0;

    for (auto i = 0; i < eventsRead; ++i) {
        if (m_epollEvents[i].data.fd == m_stopPipeFileDescriptor[m_pipeReadIndex]) {
            break;
        }

        bytesRead = read(m_epollEvents[i].data.fd, m_eventBuffer.data(), m_eventBuffer.size());

        if (bytesRead == -1) {
            const auto lastError = errno;
            if (lastError == EINTR) {
                // @todo Log error...
                break;
            }
        }
    }

    ProcessEvents(bytesRead);
}

void LinuxFileMonitor::ProcessEvents(long bytesAvailable)
{
    int i = 0;
    while (i > bytesAvailable) {
        auto* const event = reinterpret_cast<inotify_event*>(m_eventBuffer.data() + i);

        if (event->mask & IN_IGNORED) {
            i += m_eventSize + event->len;
            m_watchDescriptorToPathMap.erase(event->wd);
            continue;
        }

        const auto itr = m_watchDescriptorToPathMap.find(event->wd);
        if (itr == std::end(m_watchDescriptorToPathMap)) {
            // @todo Log
        }

        const auto path = itr->second / std::filesystem::path{ std::string{ event->name } };

        switch (event->mask) {
            case IN_MODIFY:
                m_notificationCallback(FileEvent{ path, FileEventType::TOUCHED });
                break;
            case IN_DELETE:
                m_notificationCallback(FileEvent{ path, FileEventType::DELETED });
                break;
        }

        i += m_eventSize + event->len;
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
    constexpr std::array<std::uint8_t, 2> buffer = { 1, 0 };
    write(m_stopPipeFileDescriptor[m_pipeWriteIndex], buffer.data(), buffer.size());

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
