#include "diskScanner.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>

#include <boost/filesystem.hpp>

#include "tree.h"

DiskScanner::DiskScanner()
{

}

DiskScanner::DiskScanner(const std::wstring& rawPath)
{
   boost::filesystem::path path(rawPath);

   try {
      if (!boost::filesystem::exists(path))
      {
         std::cout << "Path does not exist!" << std::endl;
         return;
      }

      m_fileTree.SetHead(path.filename().wstring());

      if (boost::filesystem::is_directory(path))
      {
         for (auto itr = boost::filesystem::directory_iterator(path);
              itr != boost::filesystem::directory_iterator();
              ++itr)
         {
            m_fileTree.GetHead()->AppendChild(itr->path().filename().wstring());
         }
      }
      else if (boost::filesystem::is_regular_file(path))
      {
         // Do nothing for now.
      }
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
   std::for_each(std::begin(m_fileTree), std::end(m_fileTree),
      [] (const TreeNode<std::wstring>& node)
   {
      std::wcout << node.GetData() << std::endl;
   });
}
