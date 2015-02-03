#include "diskScanner.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <locale>
#include <numeric>

std::uintmax_t DiskScanner::SIZE_UNDEFINED = 0;

namespace
{
   /**
    * Traverses the file tree from beginning to end, accumulating the file sizes (in bytes) of all
    * regular (non-directory, or symbolic) files.
    *
    * @param[in] fileTree           The tree to be traversed.
    * @returns The size in bytes of all files in the tree.
    */
   template<typename T>
   std::uintmax_t ComputeFileTreeSizeInBytes(const Tree<T>& fileTree)
   {
      const std::uintmax_t treeSize = std::accumulate(fileTree.beginLeaf(), fileTree.endLeaf(),
         std::uintmax_t{0}, [] (const std::uintmax_t result, const TreeNode<T>& node)
      {
         const FileInfo fileInfo = node.GetData();
         if (fileInfo.m_type == FILE_TYPE::REGULAR)
         {
            return result + fileInfo.m_size;
         }

         return result;
      });

      return treeSize;
   }
}

DiskScanner::DiskScanner()
{
}

DiskScanner::DiskScanner(const std::wstring& rawPath)
   : m_fileTree(nullptr),
     m_filesScanned(0)
{
   const boost::filesystem::path path{rawPath};
   const bool isPathValid = boost::filesystem::exists(path);
   if (!isPathValid)
   {
      throw std::invalid_argument("The provided path does not seem to exist!");
   }

   m_path = path;
}

DiskScanner::~DiskScanner()
{
}

void DiskScanner::Scan(std::atomic<std::pair<std::uintmax_t, bool>>* progress)
{
   m_fileTree = std::make_unique<Tree<FileInfo>>(Tree<FileInfo>(
      FileInfo(m_path.filename().wstring(), DiskScanner::SIZE_UNDEFINED, FILE_TYPE::DIRECTORY)
   ));

   try
   {
      ScanRecursively(m_path, *m_fileTree->GetHead(), progress);
      progress->store(std::make_pair(m_filesScanned, true));
   }
   catch (const boost::filesystem::filesystem_error& exception)
   {
      std::cout << exception.what() << std::endl;
   }
}

template<typename T>
void DiskScanner::ScanRecursively(const boost::filesystem::path& path, TreeNode<T>& fileNode,
   std::atomic<std::pair<std::uintmax_t, bool>>* progress)
{
//   {
//      std::lock_guard<std::mutex> guard(m_mutex);
//      if (m_filesScanned % 1000 == 0)
//      {
//         progress->store(std::make_pair(m_filesScanned, false));
//         //std::cout << "\r" << m_filesScanned << " files scanned as far..." << std::flush;
//      }
//   }

   if (boost::filesystem::is_regular_file(path) && !boost::filesystem::is_symlink(path))
   {
      FileInfo fileInfo(path.filename().wstring(), boost::filesystem::file_size(path),
         FILE_TYPE::REGULAR);

      fileNode.AppendChild(fileInfo);

//      {
//         std::lock_guard<std::mutex> guard(m_mutex);
//         ++m_filesScanned;
//      }
   }
   else if (boost::filesystem::is_directory(path) && !boost::filesystem::is_symlink(path))
   {
      FileInfo directoryInfo(path.filename().wstring(), DiskScanner::SIZE_UNDEFINED,
         FILE_TYPE::DIRECTORY);

      fileNode.AppendChild(directoryInfo);

//      {
//         std::lock_guard<std::mutex> guard(m_mutex);
//         ++m_filesScanned;
//      }

      for (auto itr = boost::filesystem::directory_iterator(path);
           itr != boost::filesystem::directory_iterator();
           ++itr)
      {
         boost::filesystem::path nextPath = itr->path();
         ScanRecursively(nextPath, *fileNode.GetLastChild(), progress);
      }
   }
}

void DiskScanner::ScanInNewThread(std::atomic<std::pair<std::uintmax_t, bool>>* progress)
{
   m_scanningThread = std::thread(&DiskScanner::Scan, this, progress);
}

void DiskScanner::JoinScanningThread()
{
   m_scanningThread.join();
}

std::uintmax_t DiskScanner::GetNumberOfFilesScanned()
{
   std::lock_guard<std::mutex> guard(m_mutex);
   return m_filesScanned;
}

void DiskScanner::PrintTree() const
{
   std::cout << "=============" << std::endl;
   std::cout << "  The Tree!  " << std::endl;
   std::cout << "=============" << std::endl;

   std::for_each(m_fileTree->beginPreOrder(), m_fileTree->endPreOrder(),
      [] (const TreeNode<FileInfo>& node)
   {
      const auto depth = Tree<FileInfo>::Depth(node);
      const auto tabSize = 2;
      const std::wstring padding((depth * tabSize), ' ');

      std::wcout << padding << node.GetData().m_name << std::endl;
   });
}

void DiskScanner::PrintTreeMetadata() const
{
   const std::uintmax_t sizeInBytes = ComputeFileTreeSizeInBytes(*m_fileTree);
   const double sizeInMegabytes = DiskScanner::ConvertBytesToMegaBytes(sizeInBytes);

   const unsigned int treeSize = Tree<FileInfo>::Size(*m_fileTree->GetHead());

   const auto startTime = std::chrono::high_resolution_clock::now();
   const auto fileCount = std::count_if(std::begin(*m_fileTree), std::end(*m_fileTree),
    [] (const TreeNode<FileInfo>& node)
   {
      return node.GetData().m_type == FILE_TYPE::REGULAR;
   });
   const auto endTime = std::chrono::high_resolution_clock::now();

   std::chrono::duration<double> traversalTime =
      std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime);

   std::cout.imbue(std::locale(""));

   std::cout << "=============" << std::endl;
   std::cout << "Tree Metadata" << std::endl;
   std::cout << "=============" << std::endl;

   std::cout << "File Size (Logical):" << std::endl;
   std::cout << sizeInMegabytes << " MB (" << sizeInBytes << " bytes)" << std::endl;

   std::cout << "Total Node Count:" << std::endl;
   std::cout << treeSize << std::endl;

   std::cout << "File Count:" << std::endl;
   std::cout << fileCount << std::endl;

   std::cout << "Folder Count:" << std::endl;
   std::cout << treeSize - 1 - fileCount << std::endl;

   std::cout << "Tree Traversal Time (in seconds):" << std::endl;
   std::cout << traversalTime.count() << std::endl;
}

double DiskScanner::ConvertBytesToMegaBytes(const std::uintmax_t bytes)
{
   const double oneMegabyte = std::pow(2, 20);
   return bytes / oneMegabyte;
}

double DiskScanner::ConvertBytesToGigaBytes(const std::uintmax_t bytes)
{
   const double oneGigabyte = std::pow(2, 30);
   return bytes / oneGigabyte;
}
