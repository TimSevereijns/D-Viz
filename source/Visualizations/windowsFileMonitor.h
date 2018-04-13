#ifndef WINDOWSFILEMONITOR_H
#define WINDOWSFILEMONITOR_H

#include <experimental/filesystem>

class WindowsFileMonitor
{
   public:
      WindowsFileMonitor(const std::experimental::filesystem::path& path);
};

#endif // WINDOWSFILEMONITOR_H
