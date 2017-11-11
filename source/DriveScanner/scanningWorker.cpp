#include "driveScanner.h"

#include "scanningWorker.h"

#include <spdlog/spdlog.h>
#include <Stopwatch/Stopwatch.hpp>

#include "../constants.h"
#include "../Utilities/ignoreUnused.hpp"
#include "../Utilities/threadSafeQueue.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <mutex>
#include <system_error>
#include <thread>
#include <vector>

#ifdef Q_OS_WIN
#include <windows.h>
#include <fileapi.h>
#endif

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

      IgnoreUnused(path, fileSize);

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

      const size_t nodesRemoved = toBeDeleted.size();

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
         const FileInfo fileInfo = node->file;

         Tree<VizFile>::Node* parent = node.GetParent();
         if (!parent)
         {
            return;
         }

         FileInfo& parentInfo = parent->GetData().file;
         if (parentInfo.type == FileType::DIRECTORY)
         {
            parentInfo.size += fileInfo.size;
         }
      }
   }

   /**
    * @brief Creates two vectors full of tasks in need of processing.
    *
    * @param[in] path               The initial enty path at which to start the scan.
    *
    * @returns A pair of vectors containing scannable files. The first element in the
    * pair contains the directories, and the second element contains the regular files.
    */
   std::pair<std::vector<NodeAndPath>, std::vector<NodeAndPath>>
      CreateTaskItems(const std::experimental::filesystem::path& path)
   {
      std::error_code errorCode;
      auto itr = std::experimental::filesystem::directory_iterator{ path, errorCode };
      if (errorCode)
      {
         const auto& log = spdlog::get(Constants::Logging::DEFAULT_LOG);
         log->error("Could not create directory iterator!");
         log->flush();

         return { };
      }

      std::vector<NodeAndPath> directoriesToProcess;
      std::vector<NodeAndPath> filesToProcess;

      const auto end = std::experimental::filesystem::directory_iterator{ };
      while (itr != end)
      {
         const auto path = itr->path();

         const auto fileType = std::experimental::filesystem::is_directory(path)
            ? FileType::DIRECTORY
            : FileType::REGULAR;

         constexpr std::uintmax_t fileSizeToBeComputedLater{ 0 };

         const VizFile node
         {
            FileInfo
            {
               path.filename().wstring(),
               path.extension().wstring(),
               fileSizeToBeComputedLater,
               fileType
            }
         };

         NodeAndPath nodeAndPath
         {
            std::make_unique<Tree<VizFile>::Node>(std::move(node)),
            std::move(path)
         };

         if (fileType == FileType::DIRECTORY)
         {
            directoriesToProcess.emplace_back(std::move(nodeAndPath));
         }
         else if (fileType == FileType::REGULAR)
         {
            filesToProcess.emplace_back(std::move(nodeAndPath));
         }

         ++itr;
      }

      return std::make_pair(std::move(directoriesToProcess), std::move(filesToProcess));
   }

   /**
    * @brief Puts all the scanning result pieces back together again...
    *
    * @param[in] queue              A queue containing the results of the scanning tasks.
    * @param[out] fileTree          The tree into which the scan results should be inserted.
    */
   void BuildFinalTree(
      ThreadSafeQueue<NodeAndPath>& queue,
      Tree<VizFile>& fileTree)
   {
      while (!queue.IsEmpty())
      {
         NodeAndPath nodeAndPath{ };
         const auto successfullyPopped = queue.TryPop(nodeAndPath);
         if (!successfullyPopped)
         {
            assert(false);
            break;
         }

         fileTree.GetRoot()->AppendChild(*nodeAndPath.node);
         nodeAndPath.node.release();
      }
   }
}

ScanningWorker::ScanningWorker(
   const DriveScanningParameters& parameters,
   ScanningProgress& progress)
   :
   QObject{ },
   m_parameters{ parameters },
   m_progress{ progress }
{
}

std::shared_ptr<Tree<VizFile> > ScanningWorker::CreateTreeAndRootNode()
{
   assert(std::experimental::filesystem::is_directory(m_parameters.path));
   if (!std::experimental::filesystem::is_directory(m_parameters.path))
   {
      emit ShowMessageBox("Please select a directory.");
      return nullptr;
   }

   const Block rootBlock
   {
      PrecisePoint{ },
      VisualizationModel::ROOT_BLOCK_WIDTH,
      VisualizationModel::BLOCK_HEIGHT,
      VisualizationModel::ROOT_BLOCK_DEPTH
   };

   std::experimental::filesystem::path path{ m_parameters.path };

   const FileInfo fileInfo
   {
      path.wstring(),
      /* extension = */ L"",
      ScanningWorker::SIZE_UNDEFINED,
      FileType::DIRECTORY
   };

   VizFile rootNode
   {
      fileInfo,
      rootBlock
   };

   return std::make_shared<Tree<VizFile>>(std::move(rootNode));
}

void ScanningWorker::ProcessFile(
   const std::experimental::filesystem::path& path,
   Tree<VizFile>::Node& treeNode) noexcept
{
   std::uintmax_t fileSize = ComputeFileSize(path);

   if (fileSize == 0)
   {
      return;
   }

   m_progress.bytesProcessed.fetch_add(fileSize);

   const FileInfo fileInfo
   {
      path.filename().stem().wstring(),
      path.filename().extension().wstring(),
      fileSize,
      FileType::REGULAR
   };

   treeNode.AppendChild(VizFile{ fileInfo });

   m_progress.filesScanned.fetch_add(1);
}

void ScanningWorker::ProcessDirectory(
   const std::experimental::filesystem::path& path,
   Tree<VizFile>::Node& treeNode)
{
   bool isRegularFile = false;
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
      ProcessFile(path, treeNode);
   }
   else if (std::experimental::filesystem::is_directory(path)
      && !std::experimental::filesystem::is_symlink(path))
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

      treeNode.AppendChild(VizFile{ directoryInfo });

      m_progress.directoriesScanned.fetch_add(1);

      auto itr = std::experimental::filesystem::directory_iterator{ path };
      IterateOverDirectoryAndScan(itr, *treeNode.GetLastChild());
   }
}

void ScanningWorker::IterateOverDirectoryAndScan(
   std::experimental::filesystem::directory_iterator& itr,
   Tree<VizFile>::Node& treeNode) noexcept
{
   const auto end = std::experimental::filesystem::directory_iterator{ };
   while (itr != end)
   {
      ProcessDirectory(itr->path(), treeNode);

      ++itr;
   }
}

void ScanningWorker::ProcessQueue(
   ThreadSafeQueue<NodeAndPath>& taskQueue,
   ThreadSafeQueue<NodeAndPath>& resultsQueue) noexcept
{
   while (!taskQueue.IsEmpty())
   {
      NodeAndPath nodeAndPath{ };
      const auto successfullyPopped = taskQueue.TryPop(nodeAndPath);
      if (!successfullyPopped)
      {
         assert(false);
         break;
      }

      auto startingDirectory =
         std::experimental::filesystem::directory_iterator{ nodeAndPath.path };

      IterateOverDirectoryAndScan(
         startingDirectory,
         *nodeAndPath.node);

      {
         std::lock_guard<std::mutex> lock{ streamMutex };
         IgnoreUnused(lock);

         std::wcout << "Finished scanning: \"" << nodeAndPath.path.wstring() << "\"" << std::endl;
      }

      resultsQueue.Emplace(std::move(nodeAndPath));
   }

   std::lock_guard<std::mutex> lock{ streamMutex };
   IgnoreUnused(lock);

   std::cout << "Thread " << std::this_thread::get_id() << " has finished..." << std::endl;
}

void ScanningWorker::Start()
{
   auto theTree = CreateTreeAndRootNode();
   if (!theTree)
   {
      return;
   }

   emit ProgressUpdate();

   Stopwatch<std::chrono::seconds>([&] () noexcept
   {
      auto [directories, files] = CreateTaskItems(m_parameters.path);

      ThreadSafeQueue<NodeAndPath> resultQueue;
      ThreadSafeQueue<NodeAndPath> taskQueue;

      for (auto&& directory : directories)
      {
         taskQueue.Emplace(std::move(directory));
      }

      std::vector<std::thread> scanningThreads;

      const auto numberOfThreads = std::min(
         std::thread::hardware_concurrency(),
         Constants::Concurrency::THREAD_LIMIT);

      for (auto i{ 0u }; i < numberOfThreads; ++i)
      {
         scanningThreads.emplace_back([&] () noexcept { ProcessQueue(taskQueue, resultQueue); });
      }

      for (auto&& file : files)
      {
         ProcessFile(file.path, *theTree->GetRoot());
      }

      for (auto&& thread : scanningThreads)
      {
         thread.join();
      }

      BuildFinalTree(resultQueue, *theTree);
   }, [] (const auto& elapsed, const auto& units) noexcept
   {
      spdlog::get(Constants::Logging::DEFAULT_LOG)->info(
         fmt::format("Scanned Drive in: {} {}", elapsed.count(), units));
   });

   ComputeDirectorySizes(*theTree);
   PruneEmptyFilesAndDirectories(*theTree);

   emit Finished(theTree);
}
