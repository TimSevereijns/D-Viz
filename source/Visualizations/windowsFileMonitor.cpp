#include "windowsFileMonitor.h"

#include <cassert>
#include <exception>
#include <iostream>

#include <Windows.h>
#include <FileApi.h>
#include <WinBase.h>

#include "../DriveScanner/scopedHandle.h"

namespace
{
   template<typename Source, typename Sink>
   struct AdoptConstness
   {
      using type = std::conditional_t<
         std::is_const_v<Source>,
         std::add_const_t<Sink>,
         Sink>;
   };

   template<typename Source, typename Sink>
   struct AdoptVolatility
   {
      using type = std::conditional_t<
         std::is_volatile_v<Source>,
         std::add_volatile_t<Sink>,
         Sink>;
   };

   template<typename Source, typename Sink>
   struct AdoptConstVolatility
   {
      using type = typename AdoptConstness<
         Source,
         typename AdoptVolatility<Source, Sink>::type>::type;
   };

   template<typename T>
   T* AdvancePointer(T* ptr, std::ptrdiff_t offset)
   {
      if (!ptr)
      {
         return ptr;
      }

      using ByteType = typename AdoptConstVolatility<T, unsigned char>::type;

      return reinterpret_cast<T*>(reinterpret_cast<ByteType*>(ptr) + offset);
   }
}

WindowsFileMonitor::WindowsFileMonitor(const std::experimental::filesystem::path& path)
{
   HANDLE fileHandle = CreateFile(
      path.wstring().data(),
      FILE_LIST_DIRECTORY | STANDARD_RIGHTS_READ,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
      NULL,
      OPEN_EXISTING,
      FILE_FLAG_BACKUP_SEMANTICS,
      NULL);

   if (!fileHandle || fileHandle == INVALID_HANDLE_VALUE)
   {
      std::wcout << L"Could not acquire handle to: " << path.wstring() << std::endl;

      return;
   }

   static auto fileName = std::wstring(MAX_PATH, '\0');

   constexpr auto sizeOfNotification = sizeof(FILE_NOTIFY_INFORMATION) + MAX_PATH * sizeof(wchar_t);
   std::vector<char> buffer(1024 * sizeOfNotification, '\0');

   while (true)
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

      DWORD bytesReturned{ 0 };

      const bool successfullyReadNotifications = ReadDirectoryChangesW(
         fileHandle,
         buffer.data(),
         static_cast<DWORD>(buffer.size()),
         TRUE, /* monitor the entire subtree */
         desiredNotifications,
         &bytesReturned,
         NULL,
         NULL);

      if (successfullyReadNotifications == false)
      {
         if (bytesReturned == 0 && GetLastError() == ERROR_NOTIFY_ENUM_DIR)
         {
            std::cout << "Buffer overflow detected!\n";
         }
         else if (bytesReturned == 0)
         {
            std::cout << "Buffer was either too large to allocate, or too small for the changes.\n";
         }
         else
         {
            std::cout << "Unknown error occured.\n";
         }

         std::cout << std::flush;

         continue;
      }

      assert(bytesReturned <= buffer.size());

      auto* notificationInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer.data());
      while (notificationInfo != nullptr)
      {
         if (notificationInfo->FileNameLength == 0)
         {
            notificationInfo = notificationInfo->NextEntryOffset != 0
               ? AdvancePointer(notificationInfo, notificationInfo->NextEntryOffset)
               : nullptr;

            continue;
         }

         assert(notificationInfo->FileNameLength  / sizeof(wchar_t) <= MAX_PATH);

         std::memcpy(&fileName[0],
            notificationInfo->FileName,
            notificationInfo->FileNameLength);

         fileName.resize(notificationInfo->FileNameLength / sizeof(wchar_t));

         switch (notificationInfo->Action)
         {
         case FILE_ACTION_ADDED:
            std::wcout << L"File Added: " << fileName << std::endl;
            break;

         case FILE_ACTION_REMOVED:
            std::wcout << L"File Removed: " << fileName << std::endl;
            break;

         case FILE_ACTION_MODIFIED:
            std::wcout << L"File Modified: " << fileName << std::endl;
            break;

         case FILE_ACTION_RENAMED_OLD_NAME:
            std::wcout << L"File Renamed From: " << fileName << std::endl;
            break;

         case FILE_ACTION_RENAMED_NEW_NAME:
            std::wcout << L"File Renamed To: " << fileName << std::endl;
            break;

         default:
            std::wcout << L"Unkown Action: " << fileName << std::endl;
         }

         notificationInfo = notificationInfo->NextEntryOffset != 0
            ? AdvancePointer(notificationInfo, notificationInfo->NextEntryOffset)
            : nullptr;
      }
   }

   CloseHandle(fileHandle);
}
