#include "Scanner/Monitor/linuxFileMonitor.h"

#include "constants.h"

#include <strstream>
#include <system_error>

#include <unistd.h>

#include <gsl/gsl_assert>
#include <spdlog/spdlog.h>

#ifdef Q_OS_LINUX

LinuxFileMonitor::~LinuxFileMonitor() noexcept
{
    if (m_isActive) {
        Stop();
    }
}

void LinuxFileMonitor::Start(
    const std::filesystem::path& path,
    const std::function<void(FileEvent&&)>& /*onNotificationCallback*/)
{
    m_pathToWatch = path;

    InitializeInotify();
    RegisterWatchersRecursively(path);
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
    if (!std::filesystem::exists(path)) {
        // throw
    }

    const int watchDescriptor =
        inotify_add_watch(m_inotifyFileDescriptor, path.string().c_str(), IN_ALL_EVENTS);

    if (watchDescriptor == -1) {
        const std::string lastError = strerror(errno);
        if (lastError == "28") {
            const auto message =
                "Exceeded watch limit. Edit \"/proc/sys/fs/inotify/max_user_watches\" to increase "
                "limit. Error: " +
                lastError + ".";

            throw std::runtime_error(message);
        }

        const auto message = "Failed to register watch. Error: " + lastError + ".";
        throw std::runtime_error(message);
    }

    watchDescriptorToPathMap.emplace(watchDescriptor, path);
}

void LinuxFileMonitor::Stop()
{
    m_isActive = false;

    // m_notifier.stop();

    if (m_monitoringThread.joinable()) {
        m_monitoringThread.join();
    }
}

bool LinuxFileMonitor::IsActive() const
{
    return m_isActive;
}

#endif // Q_OS_LINUX
