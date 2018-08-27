#include "windowsFileMonitor.h"

#ifdef Q_OS_WIN

#include <cassert>
#include <exception>
#include <iostream>

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

      return reinterpret_cast<DataType*>(reinterpret_cast<std::byte*>(ptr) + offset);
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

   if (m_fileHandle && m_fileHandle != INVALID_HANDLE_VALUE)
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
      /* fileName = */ path.wstring().data(),
      /* access = */ FILE_LIST_DIRECTORY | STANDARD_RIGHTS_READ,
      /* shareMode = */ FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
      /* securityAttributes = */ NULL,
      /* creationDisposition = */ OPEN_EXISTING,
      /* flagsAndAttributes = */ FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
      /* templateFile = */ NULL);

   if (!m_fileHandle || m_fileHandle == INVALID_HANDLE_VALUE)
   {
      const auto& log = spdlog::get(Constants::Logging::DEFAULT_LOG);
      log->error("Could not acquire handle to: {}.", path.string());

      assert(false);
      return;
   }

   const auto terminationHandle = CreateEventW(
      /* securityAttributes = */ NULL,
      /* manualReset = */ true,
      /* initialState = */ false,
      /* eventName = */ L"D-VIZ_FILE_MONITOR_TERMINATE_THREAD");

   m_events.SetExitHandle(terminationHandle);

   const auto notificationHandle = CreateEventW(
      /* securityAttributes = */ NULL,
      /* manualReset = */ false,
      /* initialState = */ false,
      /* eventName = */ L"D-VIZ_FILE_MONITOR_NOTIFICATION");

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
      /* directoryHandle = */ m_fileHandle,
      /* outputBuffer = */ m_notificationBuffer.data(),
      /* bufferLength = */ static_cast<DWORD>(m_notificationBuffer.size()),
      /* watchSubtree = */ TRUE,
      /* filter = */ desiredNotifications,
      /* bytesReturned = */ NULL,
      /* overlapped = */ &m_ioBuffer,
      /* callback = */ NULL);

   assert(successfullyQueued);

   const auto waitResult = WaitForMultipleObjects(
      /* handleCount = */ m_events.Size(),
      /* handles = */ m_events.Data(),
      /* awaitAll = */ false,
      /* timeout = */ INFINITE);

   switch (waitResult)
   {
      case WAIT_OBJECT_0:
      {
         m_keepMonitoring.store(false);

         CancelIo(m_fileHandle);

         while (!HasOverlappedIoCompleted(&m_ioBuffer))
         {
            SleepEx(100, TRUE);
         }

         break;
      }
      case WAIT_OBJECT_0 + 1:
      {
         RetrieveNotification();

         break;
      }
      default:
      {
         assert(false);
      }
   }
}

void WindowsFileMonitor::RetrieveNotification()
{
   DWORD bytesTransferred{ 0 };

   const bool successfullyRead = GetOverlappedResult(
      /* handle = */ m_fileHandle,
      /* overlapped = */ &m_ioBuffer,
      /* bytesTransfered = */ &bytesTransferred,
      /* wait = */ false);

   if (successfullyRead && bytesTransferred > 0)
   {
      ProcessNotification();
   }
   else
   {
      assert(false);
   }
}

void WindowsFileMonitor::ProcessNotification()
{
   auto* notificationInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(m_notificationBuffer.data());
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
      assert(fileNameLength);

      std::wstring fileName(fileNameLength, '\0');

      std::memcpy(
         &fileName[0],
         notificationInfo->FileName,         //< @todo Handle short filenames correctly.
         notificationInfo->FileNameLength);  //< Note that this length is measured in bytes!

      const auto timestamp = std::chrono::high_resolution_clock::now();

      switch (notificationInfo->Action)
      {
         case FILE_ACTION_ADDED:
         {
            m_notificationCallback(
               FileChangeNotification{ std::move(fileName), FileSystemChange::CREATED, timestamp });

            break;
         }
         case FILE_ACTION_REMOVED:
         {
            m_notificationCallback(
               FileChangeNotification{ std::move(fileName), FileSystemChange::DELETED, timestamp });

            break;
         }
         case FILE_ACTION_MODIFIED:
         {
            m_notificationCallback(
               FileChangeNotification{ std::move(fileName), FileSystemChange::MODIFIED, timestamp });

            break;
         }
         case FILE_ACTION_RENAMED_OLD_NAME:
         {
            m_pendingRenameEvent = std::move(fileName);

            break;
         }
         case FILE_ACTION_RENAMED_NEW_NAME:
         {
            assert(m_pendingRenameEvent);

            m_notificationCallback(
               FileChangeNotification{ std::move(fileName), FileSystemChange::RENAMED, timestamp });

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
