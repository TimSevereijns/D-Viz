#ifndef DISKSCANNER_H
#define DISKSCANNER_H

#include <memory>
#include <string>

#include "tree.h"

class DiskScanner
{
   public:
      explicit DiskScanner();
      explicit DiskScanner(const std::wstring& rawPpath);
      ~DiskScanner();

      void PrintTree() const;

   private:
      std::unique_ptr<Tree<std::wstring>> m_fileTree;
};

#endif // DISKSCANNER_H
