#include "driveScanner.h"

#ifdef Q_OS_WIN
   #include <FileApi.h>
   #include <Windows.h>
   #include <WinIoCtl.h>

   #include "scopedHandle.h"
   #include "winHack.hpp"
#endif

#include "scanningWorker.h"

#ifdef Q_OS_WIN
   #pragma warning(push)
   #pragma warning(disable: 4996)
#endif
   #include <boost/asio/post.hpp>
#ifdef Q_OS_WIN
   #pragma warning(pop)
#endif

#include <spdlog/spdlog.h>
#include <Stopwatch/Stopwatch.hpp>

#include "../constants.h"
#include "../Utilities/ignoreUnused.hpp"

#include <iostream>

namespace
{
   std::mutex streamMutex;

   /**
    * @brief Use the `FindFirstFileW(...)` function to retrieve the file size.
    *
    * The `std::experimental::filesystem::file_size(...)` function uses a different native function
    * to get at the file size for a given file, and this function (while probably faster than
    * `FindFirstFileW(...)`) has a tendency to throw. If such exceptional behaviour were to occur,
    * then this function can be used to hopefully still get at the file size.
    *
    * @param path[in]               The path to the troublesome file.
    *
    * @returns The size of the file if it's accessible, and zero otherwise.
    */
   std::uintmax_t GetFileSizeUsingWinAPI(const std::experimental::filesystem::path& path)
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
#endif

      return fileSize;
   }

   /**
    * @brief Helper function to safely wrap the retrieval of a file's size.
    *
    * @param path[in]               The path to the file.
    *
    * @return The size of the file if it's accessible, and zero otherwise.
    */
   std::uintmax_t ComputeFileSize(const std::experimental::filesystem::path& path) noexcept
   {
      try
      {
         assert(!std::experimental::filesystem::is_directory(path));

         return std::experimental::filesystem::file_size(path);
      }
      catch (...)
      {
         std::lock_guard<std::mutex> lock{ streamMutex };
         IgnoreUnused(lock);

         std::wcout << "Falling back on the Win API for: \"" << path.wstring() << "\"" << std::endl;
         return GetFileSizeUsingWinAPI(path);
      }
   }

   /**
    * @brief Removes nodes whose corresponding file or directory size is zero. This is often
    * necessary because a directory may contain only a single other directory within it that is
    * empty. In such a case, the outer directory has a size of zero, but
    * std::experimental::filesystem::is_empty will still have reported this directory as being
    * non-empty.
    *
    * @param[in, out] tree           The tree to be pruned.
    */
   void PruneEmptyFilesAndDirectories(Tree<VizFile>& tree)
   {
      std::vector<Tree<VizFile>::Node*> toBeDeleted;

      for (auto&& node : tree)
      {
         if (node->file.size == 0)
         {
            toBeDeleted.emplace_back(&node);
         }
      }

      const auto nodesRemoved = toBeDeleted.size();

      for (auto* node : toBeDeleted)
      {
         node->DeleteFromTree();
      }

      spdlog::get(Constants::Logging::DEFAULT_LOG)->info(
         fmt::format("Number of Sizeless Files Removed: {}", nodesRemoved)
      );
   }

   /**
    * @brief Performs a post-processing step that iterates through the tree and computes the size
    * of all directories.
    *
    * @param[in, out] tree          The tree whose nodes need their directory sizes computed.
    */
   void ComputeDirectorySizes(Tree<VizFile>& tree)
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

   /**
   * @brief Contructs the root node for the file tree.
   *
   * @param[in] path                The path to the directory that should constitute the root node.
   */
   std::shared_ptr<Tree<VizFile>> CreateTreeAndRootNode(
      const std::experimental::filesystem::path& path)
   {
      if (!std::experimental::filesystem::is_directory(path))
      {
         return nullptr;
      }

      const FileInfo fileInfo
      {
         path.wstring(),
         /* extension = */ L"",
         ScanningWorker::SIZE_UNDEFINED,
         FileType::DIRECTORY
      };

      return std::make_shared<Tree<VizFile>>(VizFile{ std::move(fileInfo) });
   }

#ifdef Q_OS_WIN
   /**
   * @returns A handle representing the repartse point found at the given path. If
   * the path is not a reparse point, then an invalid handle will be returned instead.
   */
   auto OpenReparsePoint(const std::experimental::filesystem::path& path)
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

   /**
   * @brief Reads the reparse point found at the given path into the output buffer.
   *
   * @returns True if the path could be read as a reparse point, and false otherwise.
   */
   auto ReadReparsePoint(
      const std::wstring& path,
      std::vector<std::byte>& reparseBuffer)
   {
      const auto handle = OpenReparsePoint(path);
      if (!handle.IsValid())
      {
         return false;
      }

      DWORD bytesReturned{ 0 };

      const auto successfullyRetrieved = DeviceIoControl(
         /* device = */ handle,
         /* controlCode = */ FSCTL_GET_REPARSE_POINT,
         /* inBuffer = */ NULL,
         /* inBufferSize = */ 0,
         /* outBuffer = */ reinterpret_cast<LPVOID>(reparseBuffer.data()),
         /* outBufferSize = */ static_cast<DWORD>(reparseBuffer.size()),
         /* bytesReturned = */ &bytesReturned,
         /* overlapped = */ 0) == TRUE;

      return successfullyRetrieved && bytesReturned;
   }

   /**
   * @returns True if the given file path matches the given reparse tag, and false otherwise.
   */
   auto IsReparseTag(
      const std::experimental::filesystem::path& path,
      DWORD targetTag)
   {
      static std::vector<std::byte> buffer{ MAXIMUM_REPARSE_DATA_BUFFER_SIZE };

      const auto successfullyRead = ReadReparsePoint(path, buffer);

      return successfullyRead
         ? reinterpret_cast<REPARSE_DATA_BUFFER*>(buffer.data())->ReparseTag == targetTag
         : false;
   }

   /**
   * @returns True if the given file path represents a mount point, and false otherwise.
   *
   * @note Junctions in Windows are considered mount points.
   */
   auto IsMountPoint(const std::experimental::filesystem::path& path)
   {
      const auto isMountPoint = IsReparseTag(path, IO_REPARSE_TAG_MOUNT_POINT);

      if (isMountPoint)
      {
         const std::lock_guard<decltype(streamMutex)> lock{ streamMutex };
         IgnoreUnused(lock);

         std::wcout << L"Found Mount Point: " << path.wstring() << std::endl;
      }

      return isMountPoint;
   }

   /**
   * @returns True if the given file path represents a symlink, and false otherwise.
   */
   auto IsSymlink(const std::experimental::filesystem::path& path)
   {
      const auto isSymlink = IsReparseTag(path, IO_REPARSE_TAG_SYMLINK);

      if (isSymlink)
      {
         const std::lock_guard<decltype(streamMutex)> lock{ streamMutex };
         IgnoreUnused(lock);

         std::wcout << L"Found Symlink: " << path.wstring() << std::endl;
      }

      return isSymlink;
   }

   /**
   * @returns True if the given path represents a reparse point, and false otherwise.
   */
   bool IsReparsePoint(const std::experimental::filesystem::path& path)
   {
      const ScopedHandle handle = OpenReparsePoint(path);
      if (!handle.IsValid())
      {
         return false;
      }

      BY_HANDLE_FILE_INFORMATION fileInfo = { 0 };

      const auto successfullyRetrieved = GetFileInformationByHandle(handle, &fileInfo);
      if (!successfullyRetrieved)
      {
         return false;
      }

      return fileInfo.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT;
   }
#endif

   /**
    * @returns True if the directory should be processed.
    */
   auto ShouldProcess(const std::experimental::filesystem::path& path)
   {
#ifdef Q_OS_WIN
      return !IsReparsePoint(path); //!IsSymlink(path) && !IsMountPoint(path);
#elif defined(Q_OS_LINUX)
      return std::experimental::filesystem::is_symlink(path);
#endif
   }
}

ScanningWorker::ScanningWorker(
   const DriveScanningParameters& parameters,
   ScanningProgress& progress)
   :
   m_parameters{ parameters },
   m_progress{ progress },
   m_fileTree{ CreateTreeAndRootNode(parameters.path) }
{
}

void ScanningWorker::ProcessFile(
   const std::experimental::filesystem::path& path,
   Tree<VizFile>::Node& treeNode) noexcept
{
   const auto fileSize = ComputeFileSize(path);
   if (fileSize == 0u)
   {
      return;
   }

   m_progress.bytesProcessed.fetch_add(fileSize);
   m_progress.filesScanned.fetch_add(1);

   const FileInfo fileInfo
   {
      path.filename().stem().wstring(),
      path.filename().extension().wstring(),
      fileSize,
      FileType::REGULAR
   };

   std::unique_lock<decltype(m_mutex)> lock{ m_mutex };
   treeNode.AppendChild(VizFile{ std::move(fileInfo) });
}

void ScanningWorker::ProcessDirectory(
   const std::experimental::filesystem::path& path,
   Tree<VizFile>::Node& node)
{
   auto isRegularFile{ false };
   try
   {
      // In certain cases, this function can, apparently, raise exceptions, although it
      // isn't entirely clear to me what circumstances need to exist for this to occur:
      isRegularFile = std::experimental::filesystem::is_regular_file(path);
   }
   catch (...)
   {
      return;
   }

   if (isRegularFile)
   {
      ProcessFile(path, node);
   }
   else if (std::experimental::filesystem::is_directory(path) && ShouldProcess(path))
   {
      try
      {
         // In some edge-cases, the Windows operating system doesn't allow anyone to access certain
         // directories, and attempts to do so will result in exceptional behaviour---pun intended.
         // In order to deal with these rare cases, we'll need to rely on a try-catch to keep going.
         // One example of a problematic directory in Windows 7 is: "C:\System Volume Information".
         if (std::experimental::filesystem::is_empty(path))
         {
            return;
         }
      }
      catch (...)
      {
         return;
      }

      const FileInfo directoryInfo
      {
         path.filename().wstring(),
         /* extension = */ L"",
         ScanningWorker::SIZE_UNDEFINED,
         FileType::DIRECTORY
      };

      std::unique_lock<decltype(m_mutex)> lock{ m_mutex };
      auto* const lastChild = node.AppendChild(VizFile{ std::move(directoryInfo) });
      lock.unlock();

      m_progress.directoriesScanned.fetch_add(1);

      auto itr = std::experimental::filesystem::directory_iterator{ path };
      AddDirectoriesToQueue(itr, *lastChild);
   }
}

void ScanningWorker::AddDirectoriesToQueue(
   std::experimental::filesystem::directory_iterator& itr,
   Tree<VizFile>::Node& node) noexcept
{
   const auto end = std::experimental::filesystem::directory_iterator{ };
   while (itr != end)
   {
      boost::asio::post(m_threadPool, [&, path = itr->path()] () noexcept
      {
         ProcessDirectory(path, node);
      });

      ++itr;
   }
}

void ScanningWorker::Start()
{
   emit ProgressUpdate();

   Stopwatch<std::chrono::seconds>([&] () noexcept
   {
      boost::asio::post(m_threadPool, [&] () noexcept
      {
        auto itr = std::experimental::filesystem::directory_iterator{ m_parameters.path };
        AddDirectoriesToQueue(itr, *m_fileTree->GetRoot());
      });

      m_threadPool.join();
   }, [] (const auto& elapsed, const auto& units) noexcept
   {
      spdlog::get(Constants::Logging::DEFAULT_LOG)->info(
         fmt::format("Scanned Drive in: {} {}", elapsed.count(), units));
   });

   ComputeDirectorySizes(*m_fileTree);
   PruneEmptyFilesAndDirectories(*m_fileTree);

   emit Finished(m_fileTree);
}
