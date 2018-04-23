#include "linuxFileMonitor.h"

#ifdef Q_OS_LINUX

LinuxFileMonitor::~LinuxFileMonitor()
{

}

void LinuxFileMonitor::Start(const std::experimental::filesystem::path& path)
{

}

void LinuxFileMonitor::Stop()
{

}

bool LinuxFileMonitor::IsActive() const
{
   return false;
}

boost::optional<FileAndChangeStatus> LinuxFileMonitor::FetchPendingFileChangeNotification() const
{
   return boost::none;
}

#endif // Q_OS_LINUX
