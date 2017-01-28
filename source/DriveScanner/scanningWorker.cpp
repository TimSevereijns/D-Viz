#include "driveScanner.h"
#include "scanningWorker.h"

#include "../constants.h"

#include "../ThirdParty/Stopwatch.hpp"
#include "../ThirdParty/ThreadSafeQueue.hpp"

#include "../Utilities/ignoreUnused.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <mutex>
#include <system_error>
#include <thread>
#include <vector>

namespace
{
   std::mutex streamMutex;

   /**
    * @brief PruneNodes removes nodes whose corresponding file or directory size is zero. This is
    * often necessary because a directory may contain a single other directory within it that is
    * empty. In such a case, the outer directory has a size of zero, but filesystem::is_empty
    * will still have reported this directory as being non-empty.
    *
    * @param[in, out] tree           The tree to be pruned.
    */
   void PruneEmptyFilesAndDirectories(Tree<VizNode>& tree)
   {
      std::vector<TreeNode<VizNode>*> toBeDeleted;

      for (auto& node : tree)
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

      std::cout << "Number of Sizeless Files Removed: " << nodesRemoved << std::endl;
   }

   /**
    * @brief ComputeDirectorySizes is a post-processing step that iterates through the tree and
    * computes the size of all directories.
    *
    * @param[in, out] tree          The tree whose nodes need their directory sizes computed.
    */
   void ComputeDirectorySizes(Tree<VizNode>& tree)
   {
      for (auto& node : tree)
      {
         const FileInfo fileInfo = node->file;

         TreeNode<VizNode>* parent = node.GetParent();
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
    * @brief PreprocessTargetFiles will partition the files to be scanned such that directories
    * come first, followed by the regular files. Any path that could not be accessed, or that points
    * to either an empty directory or a symbolic link, will be removed from the vector.
    *
    * @param[in] filesToProcess     All potential files to be scanned.
    *
    * @returns An iterator to the first file that is not a directory.
    */
   auto PreprocessTargetFiles(std::vector<NodeAndPath>& filesToProcess)
   {
      const auto firstNonDirectory =
         std::partition(std::begin(filesToProcess), std::end(filesToProcess),
         [] (const auto& nodeAndPath) noexcept
      {
         try
         {
            if (std::experimental::filesystem::is_directory(nodeAndPath.path)
               && !std::experimental::filesystem::is_symlink(nodeAndPath.path)
               && !std::experimental::filesystem::is_empty(nodeAndPath.path))
            {
               return true;
            }
         }
         catch (...)
         {
            return false;
         }

         return false;
      });

      const auto firstInvalidFile =
         std::partition(firstNonDirectory, std::end(filesToProcess),
         [] (const auto& nodeAndPath) noexcept
      {
         try
         {
            if (std::experimental::filesystem::is_regular_file(nodeAndPath.path))
            {
               return true;
            }
         }
         catch (...)
         {
            return false;
         }

         try
         {
            std::cout << "Whoops: " << nodeAndPath.path.string() << "\n";
            std::cout << std::experimental::filesystem::file_size(nodeAndPath.path) << std::endl;
         }
         catch (...)
         {
            std::cout << "Fuck\n";
         }

         return false;
      });

      filesToProcess.erase(firstInvalidFile, std::end(filesToProcess));

      return firstNonDirectory;
   }

   /**
    * @brief CreateTaskItems
    *
    * @param[in] path               The initial enty path at which to start the scan.
    *
    * @returns A pair of vectors containing partitioned, scannable files. The first element in the
    * pair contains the directories, and the second element contains the regular files.
    */
   std::pair<std::vector<NodeAndPath>, std::vector<NodeAndPath>>
      CreateTaskItems(const std::experimental::filesystem::path& path)
   {
      std::error_code errorCode;
      auto itr = std::experimental::filesystem::directory_iterator{ path, errorCode };
      if (errorCode)
      {
         std::cout << "Could not create directory iterator.\n";
         return { };
      }

      std::vector<NodeAndPath> filesToProcess;

      const auto end = std::experimental::filesystem::directory_iterator{ };
      while (itr != end)
      {
         // The nodes are default constructed so that we don't have to actually access any of the
         // filesystem paths yet. The nodes are fleshed out once we've had a chance to preprocess
         // the top-level files and to prune anything that might be problematic.

         auto nodeAndPath = NodeAndPath{ std::make_unique<TreeNode<VizNode>>(), itr->path() };
         filesToProcess.emplace_back(std::move(nodeAndPath));
         itr++;
      }

      const auto firstRegularFile = PreprocessTargetFiles(filesToProcess);

      for (auto& nodeAndPath : filesToProcess)
      {
         nodeAndPath.node->GetData().file.name = nodeAndPath.path.filename().wstring();
         nodeAndPath.node->GetData().file.size = std::uint64_t{ 0 };
         nodeAndPath.node->GetData().file.type = std::experimental::filesystem::is_directory(nodeAndPath.path)
            ? FileType::DIRECTORY
            : FileType::REGULAR;
      }

      std::vector<NodeAndPath> regularFiles;
      std::move(firstRegularFile, std::end(filesToProcess), std::back_inserter(regularFiles));
      filesToProcess.erase(firstRegularFile, std::end(filesToProcess));

      return std::make_pair(std::move(filesToProcess), std::move(regularFiles));
   }

   /**
    * @brief BuildFinalTree puts all the pieces back together again...
    *
    * @param[in] queue              A queue containing the results of the scanning tasks.
    * @param[out] fileTree          The tree into which the scan results should be inserted.
    */
   void BuildFinalTree(
      ThreadSafeQueue<NodeAndPath>& queue,
      Tree<VizNode>& fileTree)
   {
      while (!queue.IsEmpty())
      {
         auto nodeAndPath = NodeAndPath{ };
         const auto successfullyPopped = queue.TryPop(nodeAndPath);
         if (!successfullyPopped)
         {
            assert(false);
            continue;
         }

         fileTree.GetHead()->AppendChild(*nodeAndPath.node);
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

std::shared_ptr<Tree<VizNode>> ScanningWorker::CreateTreeAndRootNode()
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

   const VizNode rootNode
   {
      fileInfo,
      rootBlock
   };

   return std::make_shared<Tree<VizNode>>(Tree<VizNode>(rootNode));
}

void ScanningWorker::IterateOverDirectoryAndScan(
   std::experimental::filesystem::directory_iterator& itr,
   TreeNode<VizNode>& treeNode) noexcept
{
   const auto end = std::experimental::filesystem::directory_iterator{ };
   while (itr != end)
   {
      ScanRecursively(itr->path(), treeNode);

      itr++;
   }
}

void ScanningWorker::ProcessRegularFile(
   const std::experimental::filesystem::path& path,
   TreeNode<VizNode>& treeNode) noexcept
{
    const auto fileSize = std::experimental::filesystem::file_size(path);
    if (fileSize == 0)
    {
       return;
    }

    m_progress.numberOfBytesProcessed.fetch_add(fileSize);

    const FileInfo fileInfo
    {
       path.filename().stem().wstring(),
       path.filename().extension().wstring(),
       fileSize,
       FileType::REGULAR
    };

    treeNode.AppendChild(VizNode{ fileInfo });

    m_progress.filesScanned.fetch_add(1);
}

void ScanningWorker::ScanRecursively(
   const std::experimental::filesystem::path& path,
   TreeNode<VizNode>& treeNode)
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
      ProcessRegularFile(path, treeNode);
   }
   else if (std::experimental::filesystem::is_directory(path) && !std::experimental::filesystem::is_symlink(path))
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

      treeNode.AppendChild(VizNode{ directoryInfo });

      m_progress.filesScanned.fetch_add(1);

      auto itr = std::experimental::filesystem::directory_iterator{ path };
      IterateOverDirectoryAndScan(itr, *treeNode.GetLastChild());
   }
}

void ScanningWorker::ProcessQueue(
   ThreadSafeQueue<NodeAndPath>& taskQueue,
   ThreadSafeQueue<NodeAndPath>& resultsQueue) noexcept
{
   while (!taskQueue.IsEmpty())
   {
      auto nodeAndPath = NodeAndPath{ };
      const auto successfullyPopped = taskQueue.TryPop(nodeAndPath);
      if (!successfullyPopped)
      {
         continue;
      }

      IterateOverDirectoryAndScan(
         std::experimental::filesystem::directory_iterator{ nodeAndPath.path },
         *nodeAndPath.node);

      {
         std::lock_guard<std::mutex> lock{ streamMutex };
         IgnoreUnused(lock);

         std::cout
            << "Finished scanning: "
            << nodeAndPath.path.string()
            << '\n';
      }

      resultsQueue.Emplace(std::move(nodeAndPath));
   }

   std::lock_guard<std::mutex> lock{ streamMutex };
   IgnoreUnused(lock);

   std::cout
      << "Thread "
      << std::this_thread::get_id()
      << " has finished...\n";
}

void ScanningWorker::Start()
{
   auto theTree = CreateTreeAndRootNode();
   if (!theTree)
   {
      return;
   }

   emit ProgressUpdate();

   Stopwatch<std::chrono::seconds>([&]
   {
      std::pair<std::vector<NodeAndPath>, std::vector<NodeAndPath>> directoriesAndFiles =
         CreateTaskItems(m_parameters.path);

      ThreadSafeQueue<NodeAndPath> resultQueue;
      ThreadSafeQueue<NodeAndPath> taskQueue;

      for (auto& nodeAndPath : directoriesAndFiles.first)
      {
         taskQueue.Emplace(std::move(nodeAndPath));
      }

      std::vector<std::thread> scanningThreads;

      const auto numberOfThreads = std::min(std::thread::hardware_concurrency(),
         static_cast<unsigned int>(Constants::Concurrency::THREAD_LIMIT));

      for (unsigned int i{ 0 }; i < numberOfThreads; ++i)
      {
         scanningThreads.emplace_back(std::thread
         {
            [&taskQueue, &resultQueue, this] () noexcept
            {
               ProcessQueue(taskQueue, resultQueue);
            }
         });
      }

      for (auto& nodeAndPath : directoriesAndFiles.second)
      {
          std::cout << "Processing File: " << nodeAndPath.path.string() << "\n";
          ProcessRegularFile(nodeAndPath.path, *nodeAndPath.node);
      }

      for (auto& thread : scanningThreads)
      {
         thread.join();
      }

      BuildFinalTree(resultQueue, *theTree);
   }, "Scanned Drive in ");

   ComputeDirectorySizes(*theTree);
   PruneEmptyFilesAndDirectories(*theTree);

   emit Finished(theTree);
}
