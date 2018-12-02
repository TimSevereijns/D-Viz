#include "windowsFileMonitor.h"

#ifdef Q_OS_WIN

#include <exception>
#include <iostream>

#include <gsl/gsl_assert>
#include <spdlog/spdlog.h>

#include "../constants.h"
#include "../DriveScanner/scopedHandle.h"

namespace
{
   /**
    * @brief Advances a non-null pointer by a specified number of bytes.
    *
    * @param[in] ptr                The pointer value to advance.
    * @param[in] offset             The number of bytes by which to advance the pointer.
    *
    * @returns An advanced pointer if the input is not null.
    */
   template<typename DataType>
   DataType* AdvancePointer(
      DataType* ptr,
      std::ptrdiff_t offset)
   {
      if (ptr == nullptr)
      {
         return ptr;
      }

      return reinterpret_cast<DataType*>(reinterpret_cast<std::byte*>(ptr) + offset); // NOLINT
   }
}

WindowsFileMonitor::~WindowsFileMonitor() noexcept
{
   if (m_isActive)
   {
      Stop();
   }

   if (m_monitoringThread.joinable())
   {
      m_monitoringThread.join();
   }

   if (m_fileHandle && m_fileHandle != INVALID_HANDLE_VALUE) // NOLINT
   {
      CloseHandle(m_fileHandle);
   }
}

bool WindowsFileMonitor::IsActive() const
{
   return m_isActive;
}

void WindowsFileMonitor::Start(
   const std::experimental::filesystem::path& path,
   const std::function<void (FileChangeNotification&&)>& onNotificationCallback)
{
   m_notificationCallback = onNotificationCallback;

   constexpr auto sizeOfNotification = sizeof(FILE_NOTIFY_INFORMATION) + MAX_PATH * sizeof(wchar_t);

   m_notificationBuffer.resize(1024 * sizeOfNotification, std::byte{ 0 });

   m_fileHandle = CreateFileW(
      /* lpFileName = */ path.wstring().data(),
      /* dwDesiredAccess = */ FILE_LIST_DIRECTORY | STANDARD_RIGHTS_READ,
      /* dwShareMode = */ FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
      /* lpSecurityAttributes = */ nullptr,
      /* dwCreationDisposition = */ OPEN_EXISTING,
      /* dwFlagsAndAttributes = */ FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
      /* hTemplateFile = */ nullptr);

   if (!m_fileHandle || m_fileHandle == INVALID_HANDLE_VALUE) // NOLINT
   {
      const auto& log = spdlog::get(Constants::Logging::DEFAULT_LOG);
      log->error("Could not acquire handle to: {}.", path.string());

      return;
   }

   const auto exitThreadHandle = CreateEventW(
      /* lpEventAttributes = */ nullptr,
      /* bManualReset = */ true,
      /* bInitialState = */ false,
      /* lpName = */ L"D-VIZ_FILE_MONITOR_TERMINATE_THREAD");

   m_events.SetExitHandle(exitThreadHandle);

   const auto notificationHandle = CreateEventW(
      /* lpEventAttributes = */ nullptr,
      /* bManualReset = */ false,
      /* bInitialState = */ false,
      /* lpName = */ L"D-VIZ_FILE_MONITOR_NOTIFICATION");

   m_events.SetNotificationHandle(notificationHandle);

   ::ZeroMemory(&m_ioBuffer, sizeof(OVERLAPPED));
   m_ioBuffer.hEvent = notificationHandle;

   m_isActive = true;
   m_monitoringThread = std::thread{ [&] { Monitor(); } };
}

void WindowsFileMonitor::Stop()
{
   SetEvent(m_events.GetExitHandle());   
}

void WindowsFileMonitor::Monitor()
{
   while (m_keepMonitoring)
   {
      AwaitNotification();
   }

   m_isActive = false;
}

void WindowsFileMonitor::AwaitNotification()
{
   constexpr auto desiredNotifications =
      FILE_NOTIFY_CHANGE_FILE_NAME |
      FILE_NOTIFY_CHANGE_DIR_NAME |
      FILE_NOTIFY_CHANGE_ATTRIBUTES |
      FILE_NOTIFY_CHANGE_SIZE |
      FILE_NOTIFY_CHANGE_LAST_WRITE |
      FILE_NOTIFY_CHANGE_LAST_ACCESS |
      FILE_NOTIFY_CHANGE_CREATION |
      FILE_NOTIFY_CHANGE_SECURITY;

   const bool successfullyQueued = ReadDirectoryChangesW(
      /* hDirectory = */ m_fileHandle,
      /* lpBuffer = */ m_notificationBuffer.data(),
      /* nBufferLength = */ static_cast<DWORD>(m_notificationBuffer.size()),
      /* bWatchSubtree = */ TRUE,
      /* dwNotifyFilter = */ desiredNotifications,
      /* lpBytesReturned = */ nullptr,
      /* lpOverlapped = */ &m_ioBuffer,
      /* lpCompletionRoutine = */ nullptr);

   Expects(successfullyQueued);

   const auto waitResult = WaitForMultipleObjects(
      /* nCount = */ m_events.Size(),
      /* lpHandles = */ m_events.Data(),
      /* bWaitAll = */ false,
      /* dwMilliseconds = */ INFINITE);

   switch (waitResult)
   {
      case WAIT_OBJECT_0: // NOLINT
      {
         m_keepMonitoring.store(false);

         CancelIo(m_fileHandle);

         while (!HasOverlappedIoCompleted(&m_ioBuffer)) // NOLINT
         {
            SleepEx(50, TRUE);
         }

         break;
      }
      case WAIT_OBJECT_0 + 1: // NOLINT
      {
         RetrieveNotification();

         break;
      }
      default:
      {
         Expects(false);
      }
   }
}

void WindowsFileMonitor::RetrieveNotification()
{
   DWORD bytesTransferred{ 0 };

   const bool successfullyRead = GetOverlappedResult(
      /* hFile = */ m_fileHandle,
      /* lpOverlapped = */ &m_ioBuffer,
      /* lpNumberOfBytesTransferred = */ &bytesTransferred,
      /* bWait = */ false);

   if (successfullyRead && bytesTransferred > 0)
   {
      ProcessNotification();
   }
   else
   {
      Expects(false);
   }
}

void WindowsFileMonitor::ProcessNotification()
{
   auto* notificationInfo =
      reinterpret_cast<FILE_NOTIFY_INFORMATION*>(m_notificationBuffer.data()); // NOLINT

   while (notificationInfo != nullptr)
   {
      if (notificationInfo->FileNameLength == 0)
      {
         notificationInfo = notificationInfo->NextEntryOffset != 0
            ? AdvancePointer(notificationInfo, notificationInfo->NextEntryOffset)
            : nullptr;

         continue;
      }

      const auto fileNameLength = notificationInfo->FileNameLength / sizeof(wchar_t);
      Expects(fileNameLength);

      std::wstring fileName(fileNameLength, '\0');

      // @todo Handle short filenames correctly.
      std::memcpy(
         &fileName[0],
         static_cast<void*>(notificationInfo->FileName),
         notificationInfo->FileNameLength);  //< Note that this length is measured in bytes!

      const auto timestamp = std::chrono::high_resolution_clock::now();

      switch (notificationInfo->Action)
      {
         case FILE_ACTION_ADDED:
         {
            m_notificationCallback(
               FileChangeNotification{ fileName, FileModification::CREATED, timestamp });

            break;
         }
         case FILE_ACTION_REMOVED:
         {
            m_notificationCallback(
               FileChangeNotification{ fileName, FileModification::DELETED, timestamp });

            break;
         }
         case FILE_ACTION_MODIFIED:
         {
            m_notificationCallback(
               FileChangeNotification{ fileName, FileModification::TOUCHED, timestamp });

            break;
         }
         case FILE_ACTION_RENAMED_OLD_NAME:
         {
            m_pendingRenameEvent = std::move(fileName);

            break;
         }
         case FILE_ACTION_RENAMED_NEW_NAME:
         {
            Expects(m_pendingRenameEvent);

            m_notificationCallback(
               FileChangeNotification{ fileName, FileModification::RENAMED, timestamp });

            break;
         }
         default:
         {
            const auto& log = spdlog::get(Constants::Logging::DEFAULT_LOG);
            log->error("Encountered unknown file system event: {}.", notificationInfo->Action);
         }
      }

      notificationInfo = notificationInfo->NextEntryOffset != 0
         ? AdvancePointer(notificationInfo, notificationInfo->NextEntryOffset)
         : nullptr;
   }
}

#endif // Q_OS_WIN
