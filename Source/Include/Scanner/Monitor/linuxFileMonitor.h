#ifndef LINUXFILEMONITOR_H
#define LINUXFILEMONITOR_H

#include <QtGlobal>

#ifdef Q_OS_LINUX

#include "fileChangeNotification.hpp"
#include "fileMonitorBase.h"

#include <boost/filesystem/path.hpp>

#include <functional>
#include <thread>

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
        const std::experimental::filesystem::path& path,
        const std::function<void(FileChangeNotification&&)>& onNotificationCallback) override;

    /**
     * @brief Stops monitoring the file system for changes.
     */
    void Stop() override;

    /**
     * @returns True if the file system monitor is actively monitoring.
     */
    bool IsActive() const override;

  private:
    bool m_isActive{ false };

    boost::filesystem::path m_pathToWatch;

    std::thread m_monitoringThread;
};

#endif // Q_OS_UNIX

#endif // LINUXFILEMONITOR_H
