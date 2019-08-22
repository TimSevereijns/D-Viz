#include "filesystemObserverTests.h"

#include <Scanner/Monitor/fileSystemObserver.h>

#if defined(Q_OS_WIN)
#include <Scanner/Monitor/windowsFileMonitor.h>
#elif defined(Q_OS_LINUX)
#include <Scanner/Monitor/linuxFileMonitor.h>
#endif // Q_OS_LINUX

#include "testUtilities.hpp"

namespace
{
#if defined(Q_OS_WIN)
    using FileSystemMonitor = WindowsFileMonitor;
#elif defined(Q_OS_LINUX)
    using FileSystemMonitor = LinuxFileMonitor;
#endif // Q_OS_LINUX
} // namespace

void FilesystemObserverTests::initTestCase()
{
    TestUtilities::UnzipTestData("../../Tests/Data/boost-asio.zip", "../../Tests/Sandbox");
}

void FilesystemObserverTests::cleanupTestCase()
{
    std::filesystem::remove_all("../../Tests/Sandbox");
}

void FilesystemObserverTests::MonitorDeletions()
{
    std::vector<FileEvent> receivedNotifications;

    const auto onNotifications = [&](FileEvent&& notification) {
        receivedNotifications.emplace_back(std::move(notification));
    };

    std::unique_ptr<FileMonitorBase> fileMonitor = std::make_unique<FileSystemMonitor>();

    FileSystemObserver observer{ std::move(fileMonitor), "../../Tests/Sandbox" };
    observer.StartMonitoring(onNotifications);

    // @todo Deleting the path being monitored might be problematic. Verify fix, and add test case.
    std::filesystem::remove_all("../../Tests/Sandbox/asio");

    observer.StopMonitoring();

    QVERIFY(observer.IsActive() == false);
    QCOMPARE(receivedNotifications.size(), 490ul);
}

REGISTER_TEST(FilesystemObserverTests);
