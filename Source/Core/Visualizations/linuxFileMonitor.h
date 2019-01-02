#ifndef LINUXFILEMONITOR_H
#define LINUXFILEMONITOR_H

#include <QtGlobal>

#ifdef Q_OS_LINUX

#include "fileChangeNotification.hpp"
#include "fileMonitorImpl.h"

#include <boost/filesystem/path.hpp>
#include <inotify-cpp/NotifierBuilder.h>

#include <functional>

class LinuxFileMonitor : public FileMonitorImpl
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
      const std::function<void (FileChangeNotification&&)>& onNotificationCallback) override;

   /**
    * @brief Stops monitoring the file system for changes.
    */
   void Stop() override;

   /**
    * @returns True if the file system monitor is actively monitoring.
    */
   bool IsActive() const override;

private:

   bool ProcessNotification(
      const inotify::Notification& notification,
      const std::function<void (FileChangeNotification&&)>& callback) const;

   bool m_isActive{ false };

   boost::filesystem::path m_pathToWatch;

   std::thread m_monitoringThread;

   inotify::NotifierBuilder m_notifier;
};

#endif // Q_OS_UNIX

#endif // LINUXFILEMONITOR_H
