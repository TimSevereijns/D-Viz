#include "scanningWorker.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <locale>
#include <numeric>
#include <utility>

const std::uintmax_t ScanningWorker::SIZE_UNDEFINED = 0;

ScanningWorker::ScanningWorker(Tree<VizNode>* destination, std::wstring path)
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

   std::for_each(std::begin(*m_fileTree), std::end(*m_fileTree),
      [] (const TreeNode<VizNode>& node)
   {
      const FileInfo fileInfo = node.GetData().m_file;

      std::shared_ptr<TreeNode<VizNode>>& parent = node.GetParent();
      if (parent)
      {
         FileInfo& parentInfo = parent->GetData().m_file;
         if (parentInfo.m_type == FILE_TYPE::DIRECTORY)
         {
            parentInfo.m_size += fileInfo.m_size;
         }
      }
   });
}

void ScanningWorker::ScanRecursively(const boost::filesystem::path& path, TreeNode<VizNode>& treeNode)
{
   if (boost::filesystem::is_symlink(path))
   {
      return;
   }

   if (m_filesScanned % 100 == 0)
   {
      emit ProgressUpdate(m_filesScanned);
   }

   if (boost::filesystem::is_regular_file(path) && boost::filesystem::file_size(path) > 0)
   {
      FileInfo fileInfo(path.filename().wstring(), boost::filesystem::file_size(path),
         FILE_TYPE::REGULAR);
      treeNode.AppendChild(VizNode(fileInfo));

      ++m_filesScanned;
   }
   else if (boost::filesystem::is_directory(path) && !boost::filesystem::is_empty(path))
   {
      FileInfo directoryInfo(path.filename().wstring(), ScanningWorker::SIZE_UNDEFINED,
         FILE_TYPE::DIRECTORY);
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

//   const Block rootBlock
//   {
//      QVector3D(0, 0, 0),
//      Visualization::ROOT_BLOCK_WIDTH,
//      Visualization::BLOCK_HEIGHT,
//      Visualization::ROOT_BLOCK_DEPTH
//   };

//   FileInfo fileInfo{L"Dummy Root Node", ScanningWorker::SIZE_UNDEFINED, FILE_TYPE::DIRECTORY};
//   VizNode rootNode{fileInfo, rootBlock};

   //m_fileTree = std::make_shared<Tree<VizNode>>(Tree<VizNode>(rootNode));

   try
   {
      const auto start = std::chrono::high_resolution_clock::now();
      ScanRecursively(m_path, *m_fileTree->GetHead().get());
      const auto end = std::chrono::high_resolution_clock::now();

      m_scanningTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

      ComputeDirectorySizes();

      emit Finished(m_filesScanned);
   }
   catch (const boost::filesystem::filesystem_error& exception)
   {
      std::cout << exception.what() << std::endl;
   }
}

