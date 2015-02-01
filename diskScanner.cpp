#include "diskScanner.h"

#include <algorithm>
#include <iostream>
#include <locale>
#include <numeric>

namespace
{
   /**
    * TODO: Consider replacing this recursive version with an iterative implementation.
    *
    * Max path length in Windows is 260 characters, so if that includes slashes, then the maximum
    * depth of a directory or file is no more than 130, or so. Given that the default stack size
    * in MSVC is 1MB, and I only pass in references, this recursive version may be fine---maybe...
    */
   template<typename T>
   void scanDirectory(const boost::filesystem::path& path, TreeNode<T>& fileNode)
   {
      if (boost::filesystem::is_regular_file(path))
      {
         fileNode.AppendChild(path);
      }
      else if (boost::filesystem::is_directory(path))
      {
         fileNode.AppendChild(path);

         for (auto itr = boost::filesystem::directory_iterator(path);
              itr != boost::filesystem::directory_iterator();
              ++itr)
         {
            boost::filesystem::path nextPath = itr->path();
            scanDirectory(nextPath, *fileNode.GetLastChild());
         }
      }
   }

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
      const std::uintmax_t treeSize = std::accumulate(std::begin(fileTree), std::end(fileTree),
         std::uintmax_t{0}, [] (const std::uintmax_t result, const TreeNode<T>& node)
      {
         const boost::filesystem::path path = node.GetData();
         if (boost::filesystem::is_regular_file(path))
         {
            return result + boost::filesystem::file_size(node.GetData());
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
{
   const boost::filesystem::path path{rawPath};
   const bool isPathValid = boost::filesystem::exists(path);
   if (!isPathValid)
   {
      throw std::invalid_argument("The provided path does not seem to exist!");
   }

   m_fileTree = std::make_unique<Tree<boost::filesystem::path>>(
      Tree<boost::filesystem::path>{rawPath});

   try
   {
      scanDirectory(path, *m_fileTree->GetHead());
   }
   catch (const boost::filesystem::filesystem_error& exception)
   {
      std::cout << exception.what() << std::endl;
   }
}

DiskScanner::~DiskScanner()
{
}

void DiskScanner::PrintTree() const
{
   std::cout << "=============" << std::endl;
   std::cout << "  The Tree!  " << std::endl;
   std::cout << "=============" << std::endl;

   std::for_each(std::begin(*m_fileTree), std::end(*m_fileTree),
      [] (const TreeNode<boost::filesystem::path>& node)
   {
      const auto depth = Tree<boost::filesystem::path>::Depth(node);
      const auto tabSize = 2;
      const std::wstring padding((depth * tabSize), ' ');

      std::wcout << padding << node.GetData().filename().wstring() << std::endl;
   });
}

void DiskScanner::PrintTreeMetadata() const
{
   const std::uintmax_t sizeInBytes = ComputeFileTreeSizeInBytes(*m_fileTree);
   const double sizeInMegabytes = DiskScanner::ConvertBytesToMegaBytes(sizeInBytes);

   const unsigned int treeSize = Tree<boost::filesystem::path>::Size(*m_fileTree->GetHead());

   const auto fileCount = std::count_if(std::begin(*m_fileTree), std::end(*m_fileTree),
    [] (const TreeNode<boost::filesystem::path>& node)
   {
      return boost::filesystem::is_regular_file(node.GetData());
   });

   // Imbue the global local, as defined by the environment:
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
