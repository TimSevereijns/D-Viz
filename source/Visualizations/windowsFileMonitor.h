#ifndef WINDOWSFILEMONITOR_H
#define WINDOWSFILEMONITOR_H

#include <QtGlobal>

#ifdef Q_OS_WIN

#include "fileStatusChange.hpp"
#include "Utilities/threadSafeQueue.hpp"

#include "boost/optional.hpp"

#include <array>
#include <experimental/filesystem>
#include <functional>
#include <thread>
#include <vector>

#include <Windows.h>
#include <FileApi.h>
#include <WinBase.h>

namespace Detail
{
   /**
    * @brief Wrapper around the two event handles that we care when monitoring the filesystem.
    */
   class FileMonitorEventHandles
   {
      public:

         ~FileMonitorEventHandles()
         {
            for (auto& handle : m_handles)
            {
               if (handle != nullptr && handle != INVALID_HANDLE_VALUE)
               {
                  CloseHandle(handle);
                  handle = nullptr;
               }
            }
         }

         void SetExitHandle(HANDLE handle) noexcept
         {
            m_handles[0] = handle;
         }

         void SetNotificationHandle(HANDLE handle) noexcept
         {
            m_handles[1] = handle;
         }

         auto GetExitHandle() const noexcept
         {
            return m_handles[0];
         }

         auto GetNotificationHandle() const noexcept
         {
            return m_handles[1];
         }

         const auto* Data() const noexcept
         {
            return m_handles.data();
         }

         constexpr auto Size() const noexcept
         {
            return static_cast<DWORD>(m_handles.size());
         }

      private:

         std::array<HANDLE, 2> m_handles;
   };
}

/**
 * @brief The WindowsFileMonitor class
 */
class WindowsFileMonitor
{
   public:

      WindowsFileMonitor() = default;

      ~WindowsFileMonitor();

      WindowsFileMonitor(WindowsFileMonitor&& other) = delete;
      WindowsFileMonitor& operator=(WindowsFileMonitor&& other) = delete;

      WindowsFileMonitor(const WindowsFileMonitor& other) = delete;
      WindowsFileMonitor& operator=(const WindowsFileMonitor& other) = delete;

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
      boost::optional<FileAndStatusChange> FetchPendingNotifications() const;

   private:

      void Monitor();

      void AwaitNotification();
      void RetrieveNotification();
      void ProcessNotification();

      bool m_isActive{ false };

      std::atomic_bool m_keepMonitoring{ true };

      HANDLE m_fileHandle;

      Detail::FileMonitorEventHandles m_events;

      OVERLAPPED m_ioBuffer;

      std::vector<std::byte> m_notificationBuffer;

      std::thread m_monitoringThread;

      mutable ThreadSafeQueue<FileAndStatusChange> m_pendingChanges;
};

#endif // Q_OS_WIN

#endif // WINDOWSFILEMONITOR_H
