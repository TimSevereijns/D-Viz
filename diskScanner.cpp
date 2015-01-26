#include "diskScanner.h"

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
   Tree<std::wstring> fileTree;

   try {
      if (!boost::filesystem::exists(path))
      {
         std::cout << "Path does not exist!" << std::endl;
         return;
      }

      fileTree.SetHead(path.filename().wstring());

      if (boost::filesystem::is_directory(path))
      {
         for (auto itr = boost::filesystem::directory_iterator(path);
              itr != boost::filesystem::directory_iterator();
              ++itr)
         {
            fileTree.GetHead()->AppendChild(itr->path().filename().wstring());
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

