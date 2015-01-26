#ifndef DISKSCANNER_H
#define DISKSCANNER_H

#include <string>

class DiskScanner
{
   public:
      explicit DiskScanner();
      explicit DiskScanner(const std::wstring& rawPpath);
      ~DiskScanner();

};

#endif // DISKSCANNER_H
