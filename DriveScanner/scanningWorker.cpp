#include "scanningWorker.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <locale>
#include <numeric>
#include <utility>

namespace
{
   /**
    * @brief PruneNodes removes nodes whose corresponding file or directory size is zero. This is
    * often necessary because a directory may contain a single other directory within it that is
    * empty. In such a case, the outer directory has a size of zero, but boost::filesystem::is_empty
    * will still have reported this directory as being non-empty.
    *
    * @param[in/out] tree           The tree to be pruned.
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
}

const std::uintmax_t ScanningWorker::SIZE_UNDEFINED = 0;

ScanningWorker::ScanningWorker(std::shared_ptr<Tree<VizNode>> destination, std::wstring path)
   : QObject(),
     m_path(path),
     m_filesScanned(0),
     m_fileTree(destination)
{
}

ScanningWorker::~ScanningWorker()
{
   std::cout << "The worker is dead..." << std::endl;
}

void ScanningWorker::ComputeDirectorySizes()
{
   assert(m_fileTree);

   for (auto&& node : *m_fileTree)
   {
      const FileInfo fileInfo = node->file;

      std::shared_ptr<TreeNode<VizNode>>& parent = node.GetParent();
      if (parent)
      {
         FileInfo& parentInfo = parent->GetData().file;
         if (parentInfo.type == FILE_TYPE::DIRECTORY)
         {
            parentInfo.size += fileInfo.size;
         }
      }
   }
}

void ScanningWorker::ScanRecursively(const boost::filesystem::path& path, TreeNode<VizNode>& treeNode)
{
   if (m_filesScanned % 1000 == 0)
   {
      emit ProgressUpdate(m_filesScanned);
   }

   if (boost::filesystem::is_regular_file(path) && boost::filesystem::file_size(path) > 0)
   {
      const FileInfo fileInfo
      {
         path.filename().wstring(),
         boost::filesystem::file_size(path),
         FILE_TYPE::REGULAR
      };

      treeNode.AppendChild(VizNode(fileInfo));

      ++m_filesScanned;
   }
   else if (boost::filesystem::is_directory(path) && !boost::filesystem::is_empty(path)
      && !boost::filesystem::is_symlink(path))
   {
      const FileInfo directoryInfo
      {
         path.filename().wstring(),
         ScanningWorker::SIZE_UNDEFINED,
         FILE_TYPE::DIRECTORY
      };

      treeNode.AppendChild(VizNode(directoryInfo));

      ++m_filesScanned;

      for (auto itr = boost::filesystem::directory_iterator(path);
           itr != boost::filesystem::directory_iterator();
           ++itr)
      {
         const boost::filesystem::path nextPath = itr->path();
         ScanRecursively(nextPath, *treeNode.GetLastChild());
      }
   }
}

void ScanningWorker::Start()
{
   assert(boost::filesystem::is_directory(m_path));

   try
   {
      const auto start = std::chrono::high_resolution_clock::now();
      ScanRecursively(m_path, *m_fileTree->GetHead().get());
      const auto end = std::chrono::high_resolution_clock::now();

      m_scanningTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

      ComputeDirectorySizes();

      PruneEmptyFilesAndDirectories(*m_fileTree);

      emit Finished(m_filesScanned);
   }
   catch (const boost::filesystem::filesystem_error& exception)
   {
      std::cout << exception.what() << std::endl;
   }
}
