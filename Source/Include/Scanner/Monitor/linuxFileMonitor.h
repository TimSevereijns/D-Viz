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
#include <sys/inotify.h>

/**
 * @brief The LinuxFileMonitor class
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
     * @param[in] path            The root directory to watch.
     */
    void Start(
        const std::filesystem::path& path,
        const std::function<void(FileEvent&&)>& onNotificationCallback) override;

    /**
     * @brief Stops monitoring the file system for changes.
     */
    void Stop() override;

    /**
     * @returns True if the file system monitor is actively monitoring.
     */
    bool IsActive() const override;

  private:
    void InitializeInotify();
    void ShutdownInotify();

    int ReadEventBuffer();
    void ProcessEvents(int eventsToProcess);
    std::optional<FileEvent> AwaitNextEvent();

    void RegisterWatchersRecursively(const std::filesystem::path& path);
    void RegisterWatcher(const std::filesystem::path& path);

    bool m_isActive{ false };

    std::filesystem::path m_pathToWatch;

    std::thread m_monitoringThread;

    std::unordered_map<int, std::filesystem::path> watchDescriptorToPathMap;

    int m_inotifyFileDescriptor{ 0 };
    int m_epollFileDescriptor{ 0 };

    epoll_event m_inotifyEpollEvent;
    epoll_event m_stopPipeEpollEvent;
    epoll_event m_epollEvents[1024];

    int m_stopPipeFileDescriptor[2];

    constexpr static int m_maxEvents{ 4096 };
    constexpr static int m_eventSize{ sizeof(inotify_event) };
    std::vector<std::uint8_t> m_eventBuffer =
        std::vector<std::uint8_t>(m_maxEvents * (m_eventSize + 16), 0);

    std::queue<FileEvent> m_eventQueue;

    constexpr static int m_pipeReadIndex{ 0 };
    constexpr static int m_pipeWriteIndex{ 0 };
    constexpr static int m_maxEpollEvents{ 10 };
};

#endif // Q_OS_UNIX

#endif // LINUXFILEMONITOR_H
