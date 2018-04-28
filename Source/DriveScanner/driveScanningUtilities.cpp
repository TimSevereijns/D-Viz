#include "driveScanningUtilities.h"

#include <QtGlobal>

#ifdef Q_OS_WIN
   #include <FileApi.h>
   #include <WinIoCtl.h>

   #include "winHack.hpp"
#endif // Q_OS_WIN

#include "../DataStructs/vizBlock.h"
#include "../Utilities/ignoreUnused.hpp"

#include <cassert>
#include <iostream>
#include <memory>
#include <mutex>

#include <Tree/Tree.hpp>

namespace DriveScanning
{
   namespace Detail
   {
      std::mutex streamMutex;

      ScopedHandle OpenReparsePoint(
         const std::experimental::filesystem::path& path) noexcept
      {
         const auto handle = CreateFile(
            /* fileName = */ path.wstring().c_str(),
            /* desiredAccess = */ GENERIC_READ,
            /* shareMode = */ 0,
            /* securityAttributes = */ 0,
            /* creationDisposition = */ OPEN_EXISTING,
            /* flagsAndAttributes = */ FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
            /* templateFile = */ 0);

         return ScopedHandle{ handle };
      }

      auto ReadReparsePoint(
         const std::wstring& path,
         std::vector<std::byte>& reparseBuffer) noexcept -> bool
      {
         const auto handle = OpenReparsePoint(path);
         if (!handle.IsValid())
         {
            return false;
         }

         DWORD bytesReturned{ 0 };

         const auto successfullyRetrieved = DeviceIoControl(
            /* device = */ static_cast<HANDLE>(handle),
            /* controlCode = */ FSCTL_GET_REPARSE_POINT,
            /* inBuffer = */ NULL,
            /* inBufferSize = */ 0,
            /* outBuffer = */ reinterpret_cast<LPVOID>(reparseBuffer.data()),
            /* outBufferSize = */ static_cast<DWORD>(reparseBuffer.size()),
            /* bytesReturned = */ &bytesReturned,
            /* overlapped = */ 0) == TRUE;

         return successfullyRetrieved && bytesReturned;
      }

      std::uintmax_t GetFileSizeUsingWinAPI(
         const std::experimental::filesystem::path& path) noexcept
      {
         std::uintmax_t fileSize{ 0 };

         IgnoreUnused(path);

#ifdef Q_OS_WIN

         WIN32_FIND_DATA fileData;
         const HANDLE fileHandle = FindFirstFileW(path.wstring().data(), &fileData);
         if (fileHandle == INVALID_HANDLE_VALUE)
         {
            return 0;
         }

         const auto highWord = static_cast<std::uintmax_t>(fileData.nFileSizeHigh);
         fileSize = (highWord << sizeof(fileData.nFileSizeLow) * 8) | fileData.nFileSizeLow;

#endif // Q_OS_WIN

         return fileSize;
      }
   }

   namespace Utilities
   {
      std::uintmax_t ComputeFileSize(
         const std::experimental::filesystem::path& path) noexcept
      {
         try
         {
            assert(!std::experimental::filesystem::is_directory(path));

            return std::experimental::filesystem::file_size(path);
         }
         catch (...)
         {
            std::lock_guard<std::mutex> lock{ Detail::streamMutex };
            IgnoreUnused(lock);

            std::wcout
               << "Falling back on the Win API for: \"" << path.wstring() << "\""
               << std::endl;

            return Detail::GetFileSizeUsingWinAPI(path);
         }
      }

      /**
       * @brief Performs a post-processing step that iterates through the tree and computes the size
       * of all directories.
       *
       * @param[in, out] tree          The tree whose nodes need their directory sizes computed.
       */
      void ComputeDirectorySizes(Tree<VizBlock>& tree) noexcept
      {
         for (auto&& node : tree)
         {
            const auto& fileInfo = node->file;

            auto* parent = node.GetParent();
            if (!parent)
            {
               return;
            }

            auto& parentInfo = parent->GetData().file;
            if (parentInfo.type == FileType::DIRECTORY)
            {
               parentInfo.size += fileInfo.size;
            }
         }
      }

#ifdef Q_OS_WIN

      bool IsReparseTag(
         const std::experimental::filesystem::path& path,
         DWORD targetTag) noexcept
      {
         static std::vector<std::byte> buffer{ MAXIMUM_REPARSE_DATA_BUFFER_SIZE };

         const auto successfullyRead = Detail::ReadReparsePoint(path, buffer);

         return successfullyRead
            ? reinterpret_cast<REPARSE_DATA_BUFFER*>(buffer.data())->ReparseTag == targetTag
            : false;
      }

      bool IsMountPoint(const std::experimental::filesystem::path& path) noexcept
      {
         const auto isMountPoint = IsReparseTag(path, IO_REPARSE_TAG_MOUNT_POINT);
         if (isMountPoint)
         {
            const std::lock_guard<decltype(Detail::streamMutex)> lock{ Detail::streamMutex };
            IgnoreUnused(lock);

            std::wcout << L"Found Mount Point: " << path.wstring() << std::endl;
         }

         return isMountPoint;
      }

      bool IsSymlink(const std::experimental::filesystem::path& path) noexcept
      {
         const auto isSymlink = IsReparseTag(path, IO_REPARSE_TAG_SYMLINK);
         if (isSymlink)
         {
            const std::lock_guard<decltype(Detail::streamMutex)> lock{ Detail::streamMutex };
            IgnoreUnused(lock);

            std::wcout << L"Found Symlink: " << path.wstring() << std::endl;
         }

         return isSymlink;
      }

      bool IsReparsePoint(const std::experimental::filesystem::path& path) noexcept
      {
         const auto handle = Detail::OpenReparsePoint(path);
         if (!handle.IsValid())
         {
            return false;
         }

         BY_HANDLE_FILE_INFORMATION fileInfo = { 0 };

         const auto successfullyRetrieved
            = GetFileInformationByHandle(static_cast<HANDLE>(handle), &fileInfo);

         if (!successfullyRetrieved)
         {
            return false;
         }

         return fileInfo.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT;
      }

#endif // Q_OS_WIN
   }
}


