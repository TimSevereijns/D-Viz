#ifndef LINUXFILEMONITOR_H
#define LINUXFILEMONITOR_H

#include <QtGlobal>

#ifdef Q_OS_LINUX

#include "fileStatusChange.hpp"
#include "Utilities/threadSafeQueue.hpp"

#include "boost/optional.hpp"

#include <functional>

class LinuxFileMonitor
{
   public:

      LinuxFileMonitor() = default;

      ~LinuxFileMonitor();

      LinuxFileMonitor(LinuxFileMonitor&& other) = delete;
      LinuxFileMonitor& operator=(LinuxFileMonitor&& other) = delete;

      LinuxFileMonitor(const LinuxFileMonitor& other) = delete;
      LinuxFileMonitor& operator=(const LinuxFileMonitor& other) = delete;

      /**
       * @brief Starts monitoring the file system for changes.
       *
       * @param[in] path            The root directory to watch.
       */
      void Start(
         const std::experimental::filesystem::path& path,
         const std::function<void (FileChangeNotification&&)>& onNotificationCallback);

      /**
       * @brief Stops monitoring the file system for changes.
       */
      void Stop();

      /**
       * @returns True if the file system monitor is actively monitoring.
       */
      bool IsActive() const;


   private:

      bool m_isActive{ false };
};

#endif // Q_OS_UNIX

#endif // LINUXFILEMONITOR_H
