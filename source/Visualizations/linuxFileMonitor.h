#ifndef LINUXFILEMONITOR_H
#define LINUXFILEMONITOR_H

#include <QtGlobal>

#ifdef Q_OS_LINUX

#include "fileStatusChange.hpp"
#include "Utilities/threadSafeQueue.hpp"

#include "boost/optional.hpp"

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
      void Start(const std::experimental::filesystem::path& path);

      /**
       * @brief Stops monitoring the file system for changes.
       */
      void Stop();

      /**
       * @returns True if the file system monitor is actively monitoring.
       */
      bool IsActive() const;

      /**
       * @brief Fetches the oldest, pending file change notification that hasn't yet been processed
       * by the UI.
       *
       * @return The pending change, if it exists.
       */
      boost::optional<FileAndChangeStatus> FetchPendingFileChangeNotification() const;

   private:

      bool m_isActive{ false };

      mutable ThreadSafeQueue<FileAndChangeStatus> m_pendingChanges;
};

#endif // Q_OS_UNIX

#endif // LINUXFILEMONITOR_H
