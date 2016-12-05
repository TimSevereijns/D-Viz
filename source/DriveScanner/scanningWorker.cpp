#include "driveScanner.h"
#include "scanningWorker.h"

#include "../ThirdParty/stopwatch.hpp"
#include "../ThirdParty/ThreadSafeQueue.hpp"

#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace
{
   /**
    * @brief PruneNodes removes nodes whose corresponding file or directory size is zero. This is
    * often necessary because a directory may contain a single other directory within it that is
    * empty. In such a case, the outer directory has a size of zero, but boost::filesystem::is_empty
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
    * @brief The NodeAndPath struct
    */
   struct NodeAndPath
   {
      std::unique_ptr<TreeNode<VizNode>> node;
      boost::filesystem::path path;

      NodeAndPath(
         decltype(node) node,
         decltype(path) path)
         :
         node{ std::move(node) },
         path{ std::move(path) }
      {
      }

      NodeAndPath() = default;
   };

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
            if (boost::filesystem::is_directory(nodeAndPath.path)
               && !boost::filesystem::is_symlink(nodeAndPath.path)
               && !boost::filesystem::is_empty(nodeAndPath.path))
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
            if (boost::filesystem::is_regular_file(nodeAndPath.path))
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

      filesToProcess.erase(firstInvalidFile, std::end(filesToProcess));

      return firstNonDirectory;
   }

   /**
    * @brief CreateTaskItems
    *
    * @param[in] path               The initial enty path at which to start the scan.
    *
    * @returns A vector of partitioned, scannable files. Directories first, regular files second.
    */
   std::vector<NodeAndPath> CreateTaskItems(const boost::filesystem::path& path)
   {
      boost::system::error_code errorCode;
      auto itr = boost::filesystem::directory_iterator{ path, errorCode };
      if (errorCode)
      {
         std::cout << "Could not create directory iterator." << std::endl;
         return {};
      }

      std::vector<NodeAndPath> filesToProcess;

      const auto end = boost::filesystem::directory_iterator{ };
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
         nodeAndPath.node->GetData().file.type = boost::filesystem::is_directory(nodeAndPath.path)
            ? FileType::DIRECTORY
            : FileType::REGULAR;
      }

      std::vector<NodeAndPath> regularFiles;
      std::move(firstRegularFile, std::end(filesToProcess), std::back_inserter(regularFiles));
      filesToProcess.erase(firstRegularFile, std::end(filesToProcess));

      return filesToProcess;
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
   assert(boost::filesystem::is_directory(m_parameters.path));
   if (!boost::filesystem::is_directory(m_parameters.path))
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

   boost::filesystem::path rawPath{ m_parameters.path };
   auto sanitizedPath = rawPath.remove_trailing_separator();

   const FileInfo fileInfo
   {
      sanitizedPath.wstring(),
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
   boost::filesystem::directory_iterator& itr,
   TreeNode<VizNode>& treeNode) noexcept
{
   const auto end = boost::filesystem::directory_iterator{ };
   while (itr != end)
   {
      ScanRecursively(itr->path(), treeNode);

      itr++;
   }
}

void ScanningWorker::ScanRecursively(
   const boost::filesystem::path& path,
   TreeNode<VizNode>& treeNode)
{
   bool isRegularFile = false;
   try
   {
      // In certain cases, this function can, apparently, raise exceptions, although it
      // isn't entirely clear to me what circumstances need to exist for this to occur:
      isRegularFile = boost::filesystem::is_regular_file(path);
   }
   catch (...)
   {
      return;
   }

   if (isRegularFile)
   {
      const auto fileSize = boost::filesystem::file_size(path);
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
   else if (boost::filesystem::is_directory(path) && !boost::filesystem::is_symlink(path))
   {
      try
      {
         // In some edge-cases, the Windows operating system doesn't allow anyone to access certain
         // directories, and attempts to do so will result in exceptional behaviour---pun intended.
         // In order to deal with these rare cases, we'll need to rely on a try-catch to keep going.
         // One example of a problematic directory in Windows 7 is: "C:\System Volume Information".
         if (boost::filesystem::is_empty(path))
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

      auto itr = boost::filesystem::directory_iterator{ path };
      IterateOverDirectoryAndScan(itr, *treeNode.GetLastChild());
   }
}

void ScanningWorker::Start()
{
   auto theTree = CreateTreeAndRootNode();
   if (!theTree)
   {
      return;
   }

   emit ProgressUpdate(0, 0);

   Stopwatch<std::chrono::seconds>([&]
   {
      std::vector<NodeAndPath> filesToProcess = CreateTaskItems(m_parameters.path);

      ThreadSafeQueue<NodeAndPath> taskQueue;
      for (auto& nodeAndPair : filesToProcess)
      {
         taskQueue.Emplace(std::move(nodeAndPair));
      }

      ThreadSafeQueue<NodeAndPath> resultsQueue;

      std::vector<std::thread> scanningThreads;
      std::mutex streamMutex;

      const auto numberOfThreads = std::thread::hardware_concurrency();

      for (unsigned int i = 0; i < numberOfThreads; i++)
      {
         scanningThreads.emplace_back(std::thread
         {
            [&] () noexcept
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
                     boost::filesystem::directory_iterator{ nodeAndPath.path },
                     *nodeAndPath.node);

                  {
                     std::lock_guard<std::mutex> lock{ streamMutex };
                     std::cout
                        << "Finished scanning: "
                        << nodeAndPath.path.string()
                        << '\n';
                  }

                  resultsQueue.Emplace(std::move(nodeAndPath));
               }

               std::lock_guard<std::mutex> lock{ streamMutex };
               std::cout
                  << "Thread "
                  << std::this_thread::get_id()
                  << " has finished...\n";
            }
         });
      }

      for (auto& thread : scanningThreads)
      {
         thread.join();
      }

      BuildFinalTree(resultsQueue, *theTree);
   }, "Scanned Drive in ");

   ComputeDirectorySizes(*theTree);
   PruneEmptyFilesAndDirectories(*theTree);

   emit Finished(m_progress.filesScanned.load(), theTree);
}
