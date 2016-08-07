#include "driveScanner.h"
#include "scanningWorker.h"

#include "../ThirdParty/stopwatch.hpp"

#include <iostream>

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

   constexpr std::chrono::seconds UPDATE_FREQUENCY{ 1 };
}

const std::uintmax_t ScanningWorker::SIZE_UNDEFINED = 0;

ScanningWorker::ScanningWorker(const DriveScanningParameters& parameters) :
   QObject{ },
   m_parameters{ parameters }
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

void ScanningWorker::IterateOverDirectory(
   boost::filesystem::directory_iterator& itr,
   TreeNode<VizNode>& treeNode) noexcept
{
   const auto end = boost::filesystem::directory_iterator{ };
   while (itr != end)
   {
      ScanRecursively(itr->path(), treeNode);

      boost::system::error_code errorCode;
      itr.increment(errorCode);
      if (errorCode)
      {
         emit ShowMessageBox("Could not advance iterator!");
      }
   }
}

void ScanningWorker::ScanRecursively(
   const boost::filesystem::path& path,
   TreeNode<VizNode>& treeNode)
{
   const auto now = std::chrono::high_resolution_clock::now();
   const auto timeSinceLastProgressUpdate =
      std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastProgressUpdate);

   if (timeSinceLastProgressUpdate > UPDATE_FREQUENCY)
   {
      emit ProgressUpdate(m_filesScanned);

      m_lastProgressUpdate = std::chrono::high_resolution_clock::now();
   }

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

   if (isRegularFile && boost::filesystem::file_size(path) > 0)
   {
      const FileInfo fileInfo
      {
         path.filename().stem().wstring(),
         path.filename().extension().wstring(),
         boost::filesystem::file_size(path),
         FileType::REGULAR
      };

      treeNode.AppendChild(VizNode{ fileInfo });

      ++m_filesScanned;
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

      ++m_filesScanned;

      boost::system::error_code errorCode;
      auto itr = boost::filesystem::directory_iterator{ path, errorCode };
      if (errorCode)
      {
         emit ShowMessageBox("Could not create iterator!");
         return;
      }

      IterateOverDirectory(itr, *treeNode.GetLastChild());
   }
}

void ScanningWorker::Start()
{
   auto theTree = CreateTreeAndRootNode();
   if (!theTree)
   {
      return;
   }

   emit ProgressUpdate(0);

   m_lastProgressUpdate = std::chrono::high_resolution_clock::now();

   Stopwatch<std::chrono::seconds>([&]
   {
      boost::system::error_code errorCode;
      auto itr = boost::filesystem::directory_iterator{ m_parameters.path, errorCode };
      if (errorCode)
      {
         emit ShowMessageBox("Could not create iterator!");
         return;
      }

      IterateOverDirectory(itr, *theTree->GetHead());
   }, "Scanned Drive in ");

   ComputeDirectorySizes(*theTree);
   PruneEmptyFilesAndDirectories(*theTree);

   emit Finished(m_filesScanned, theTree);
}
