#include "linuxFileMonitor.h"

#ifdef Q_OS_LINUX

LinuxFileMonitor::~LinuxFileMonitor()
{

}

void LinuxFileMonitor::Start(
   const std::experimental::filesystem::path& path,
   const std::function<void (FileChangeNotification&&)>& onNotificationCallback)
{

}

void LinuxFileMonitor::Stop()
{

}

bool LinuxFileMonitor::IsActive() const
{
   return false;
}

#endif // Q_OS_LINUX
