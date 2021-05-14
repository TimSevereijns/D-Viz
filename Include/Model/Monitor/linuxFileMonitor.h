#ifndef LINUXFILEMONITOR_H
#define LINUXFILEMONITOR_H

#include <QtGlobal>

#ifdef Q_OS_LINUX

#include "fileMonitorBase.h"

#include <filesystem>
#include <functional>
#include <queue>
#include <thread>

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/inotify.h>

/**
 * @brief A Linux-specific file monitor.
 *
 * @note Inspired by: https://github.com/erikzenker/inotify-cpp
 */
class LinuxFileMonitor : public FileMonitorBase
{
  public:
    ~LinuxFileMonitor() noexcept override;

    /**
     * @brief Starts monitoring the file system for changes.
     *
     * @param[in] path                      The root directory to watch.
     * @param[in] onNotificationCallback    The callback to invoke when an event occurs.
     */
    void Start(
        const std::filesystem::path& path,
        std::function<void(FileEvent&&)> onNotificationCallback) override;

    /**
     * @brief Stops monitoring the file system for changes.
     */
    void Stop() override;

    /**
     * @returns True if the file system monitor is actively monitoring.
     */
    bool IsActive() const override;

  private:
    void Monitor();

    void InitializeInotify();
    void CleanUpInotify() noexcept;

    void AwaitNotification();
    void ProcessEvents(long eventsToProcess);

    void RegisterWatchersRecursively(const std::filesystem::path& path);
    void RegisterWatcher(const std::filesystem::path& path);

    std::thread m_monitoringThread;

    std::atomic_bool m_isActive = false;
    std::atomic_bool m_keepMonitoring = true;

    std::filesystem::path m_pathToWatch;

    std::unordered_map<int, std::filesystem::path> m_watchDescriptorToPathMap;

    std::function<void(FileEvent&&)> m_notificationCallback;

    int m_inotifyFileDescriptor = 0;
    int m_epollFileDescriptor = 0;
    int m_stopEventFileDescriptor = 0;

    epoll_event m_inotifyEpollEvent;
    epoll_event m_stopEpollEvent;
    epoll_event m_epollEvents[1024];

    constexpr static int m_maxEpollEvents = 10;
    constexpr static int m_maxEvents = 4096;
    constexpr static int m_eventSize = sizeof(inotify_event);

    std::vector<std::uint8_t> m_eventBuffer =
        std::vector<std::uint8_t>(m_maxEvents * (m_eventSize + 16), 0);
};

#endif // Q_OS_UNIX

#endif // LINUXFILEMONITOR_H
