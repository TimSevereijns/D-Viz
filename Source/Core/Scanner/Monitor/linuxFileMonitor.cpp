#include "Scanner/Monitor/linuxFileMonitor.h"
#include "constants.h"

#include <gsl/gsl_assert>
#include <spdlog/spdlog.h>

#ifdef Q_OS_LINUX

LinuxFileMonitor::~LinuxFileMonitor() noexcept
{
    if (m_isActive) {
        Stop();
    }
}

void LinuxFileMonitor::Start(
    const std::experimental::filesystem::path& path,
    const std::function<void(FileChangeNotification&&)>& /*onNotificationCallback*/)
{
    m_pathToWatch = path.native();

    // @todo
}

void LinuxFileMonitor::Stop()
{
    m_isActive = false;

    // m_notifier.stop();

    if (m_monitoringThread.joinable()) {
        m_monitoringThread.join();
    }
}

bool LinuxFileMonitor::IsActive() const
{
    return m_isActive;
}

#endif // Q_OS_LINUX
