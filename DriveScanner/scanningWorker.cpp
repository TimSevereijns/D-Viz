#include "driveScanner.h"
#include "scanningWorker.h"

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
      std::cout << "Nodes before pruning: " << tree.Size(*tree.GetHead()) << std::endl;

      unsigned int nodesRemoved = 0;
      for (auto&& node : tree)
      {
         if (node->file.size == 0)
         {
            node.RemoveFromTree();
            nodesRemoved++;
         }
      }

      std::cout << "Nodes removed: " << nodesRemoved << std::endl;
      std::cout << "Nodes after pruning: " << tree.Size(*tree.GetHead()) << std::endl;
   }

   /**
    * @brief ComputeDirectorySizes is a post-processing step that iterates through the tree and
    * computes the size of all directories.
    *
    * @param[in, out] tree          The tree whose nodes need their directory sizes computed.
    */
   void ComputeDirectorySizes(Tree<VizNode>& tree)
   {
      for (auto&& node : tree)
      {
         const FileInfo fileInfo = node->file;

         std::shared_ptr<TreeNode<VizNode>>& parent = node.GetParent();
         if (!parent)
         {
            return;
         }

         FileInfo& parentInfo = parent->GetData().file;
         if (parentInfo.type == FILE_TYPE::DIRECTORY)
         {
            parentInfo.size += fileInfo.size;
         }
      }
   }
}

const std::uintmax_t ScanningWorker::SIZE_UNDEFINED = 0;

ScanningWorker::ScanningWorker(const DriveScanningParameters& parameters)
   : QObject(),
     m_parameters(parameters),
     m_filesScanned(0)
{
}

ScanningWorker::~ScanningWorker()
{
   std::cout << "The worker is dead..." << std::endl;
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
      DoublePoint3D{},
      Visualization::ROOT_BLOCK_WIDTH,
      Visualization::BLOCK_HEIGHT,
      Visualization::ROOT_BLOCK_DEPTH
   };

   const FileInfo fileInfo
   {
      m_parameters.path,
      ScanningWorker::SIZE_UNDEFINED,
      FILE_TYPE::DIRECTORY
   };

   const VizNode rootNode
   {
      fileInfo,
      rootBlock
   };

   return std::make_shared<Tree<VizNode>>(Tree<VizNode>{rootNode});
}

void ScanningWorker::IterateOverDirectory(
   boost::filesystem::directory_iterator& itr,
   TreeNode<VizNode>& treeNode)
{
   const auto end = boost::filesystem::directory_iterator{};
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
   using namespace std::chrono;
   const auto timeSinceLastProgressUpdate =
      duration_cast<milliseconds>(high_resolution_clock::now() - m_lastProgressUpdate).count();

   if (timeSinceLastProgressUpdate > 1000)
   {
      emit ProgressUpdate(m_filesScanned);

      m_lastProgressUpdate = high_resolution_clock::now();
   }

   if (boost::filesystem::is_regular_file(path) && boost::filesystem::file_size(path) > 0)
   {
      const FileInfo fileInfo
      {
         path.filename().wstring(),
         boost::filesystem::file_size(path),
         FILE_TYPE::REGULAR
      };

      treeNode.AppendChild(VizNode{fileInfo});

      ++m_filesScanned;
   }
   else if (boost::filesystem::is_directory(path) && !boost::filesystem::is_symlink(path))
   {
      try
      {
         // In some edge-cases, the Windows operating system doesn't allow anyone to access certain
         // directories, and attempts to do so will result in exceptional behaviour---pun intended.
         // In order to deal with these rare cases, we'll need to rely on a try-catch to keep going.
         // One example of a problematic directory in Windows 7 is: C:\System Volume Information
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
         ScanningWorker::SIZE_UNDEFINED,
         FILE_TYPE::DIRECTORY
      };

      treeNode.AppendChild(VizNode{directoryInfo});

      ++m_filesScanned;

      boost::system::error_code errorCode;
      auto itr = boost::filesystem::directory_iterator{path, errorCode};
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

   const auto startTime = std::chrono::high_resolution_clock::now();
   m_lastProgressUpdate = startTime;

   boost::system::error_code errorCode;
   auto itr = boost::filesystem::directory_iterator{m_parameters.path, errorCode};
   if (errorCode)
   {
      emit ShowMessageBox("Could not create iterator!");
      return;
   }

   IterateOverDirectory(itr, *theTree->GetHead());

   const auto endTime = std::chrono::high_resolution_clock::now();
   m_scanningTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

   ComputeDirectorySizes(*theTree);
   PruneEmptyFilesAndDirectories(*theTree);

   emit Finished(m_filesScanned, theTree);
}
