#include "diskScanner.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>

#include <boost/filesystem.hpp>

#include "tree.h"

namespace
{
   template<typename T>
   void scanDirectory(const boost::filesystem::path& path, TreeNode<T>& fileNode)
   {
      if (boost::filesystem::is_regular_file(path))
      {
         fileNode.AppendChild(path.filename().wstring());
      }
      else if (boost::filesystem::is_directory(path))
      {
         fileNode.AppendChild(path.filename().wstring());

         for (auto itr = boost::filesystem::directory_iterator(path);
              itr != boost::filesystem::directory_iterator();
              ++itr)
         {
            boost::filesystem::path nextPath = itr->path();
            scanDirectory(nextPath, *fileNode.GetLastChild());
         }
      }
   }
}

DiskScanner::DiskScanner()
{
}

DiskScanner::DiskScanner(const std::wstring& rawPath)
   : m_fileTree(std::make_unique<Tree<std::wstring>>(Tree<std::wstring>(L"Root")))
{
   boost::filesystem::path path{rawPath};

   try {
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
   std::for_each(std::begin(*m_fileTree), std::end(*m_fileTree),
      [] (const TreeNode<std::wstring>& node)
   {
      std::wcout << node.GetData() << std::endl;
   });
}
