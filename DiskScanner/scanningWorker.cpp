#include "scanningWorker.h"

#include <algorithm>
#include <chrono>
#include <locale>
#include <numeric>

#include "../DataStructs/block.h"

//const std::uintmax_t DiskScanner::SIZE_UNDEFINED = 0;

//namespace
//{
//   /**
//    * Traverses the file tree from beginning to end, accumulating the file sizes (in bytes) of all
//    * regular (non-directory, or symbolic) files.
//    *
//    * @param[in] fileTree           The tree to be traversed.
//    * @returns The size in bytes of all files in the tree.
//    */
//   std::uintmax_t ComputeTopLevelDirectorySizeInBytesViaTraversal(const Tree<VizNode>& fileTree)
//   {
//      const std::uintmax_t treeSize = std::accumulate(fileTree.beginLeaf(), fileTree.endLeaf(),
//         std::uintmax_t{0}, [] (const std::uintmax_t result, const TreeNode<VizNode>& node)
//      {
//         const FileInfo fileInfo = node.GetData().m_file;
//         if (fileInfo.m_type == FILE_TYPE::REGULAR)
//         {
//            return result + fileInfo.m_size;
//         }

//         return result;
//      });

//      return treeSize;
//   }
//}


ScanningWorker::ScanningWorker(QObject *parent) : QObject(parent)
{

}

ScanningWorker::~ScanningWorker()
{
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

//   // Dummy root node:
//   FileInfo fileInfo{L"Dummy Root Node", DiskScanner::SIZE_UNDEFINED, FILE_TYPE::DIRECTORY};
//   VizNode rootNode{fileInfo, rootBlock};

//   m_fileTree = std::make_shared<Tree<VizNode>>(Tree<VizNode>(rootNode));

//   try
//   {
//      const auto start = std::chrono::high_resolution_clock::now();
//      ScanRecursively(m_path, *m_fileTree->GetHead(), progress);
//      const auto end = std::chrono::high_resolution_clock::now();

//      m_scanningTime = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);

//      progress->store(std::make_pair(m_filesScanned, /*isScanningDone =*/ true));

//      ComputeDirectorySizes();
//   }
//   catch (const boost::filesystem::filesystem_error& exception)
//   {
//      std::cout << exception.what() << std::endl;
//   }
}

