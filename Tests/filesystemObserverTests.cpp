#include "filesystemObserverTests.h"

#include <Model/Monitor/fileSystemObserver.h>

#if defined(Q_OS_WIN)
#include <Model/Monitor/windowsFileMonitor.h>
#elif defined(Q_OS_LINUX)
#include <Model/Monitor/linuxFileMonitor.h>
#endif // Q_OS_LINUX

#include "Utilities/testUtilities.h"

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
    TestUtilities::UnzipTestData(
        std::filesystem::absolute("../../Tests/Data/boost-asio.zip"),
        std::filesystem::absolute("../../Tests/Sandbox"));
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

    FileSystemObserver observer{ std::make_unique<FileSystemMonitor>(), "../../Tests/Sandbox" };
    observer.StartMonitoring(onNotifications);
    QVERIFY(observer.IsActive() == true);

    // @todo Deleting the path being monitored might be problematic. Verify fix, and add test case.
    std::filesystem::remove_all("../../Tests/Sandbox/asio");
    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    observer.StopMonitoring();
    QVERIFY(observer.IsActive() == false);

    QCOMPARE(receivedNotifications.size(), 490ul);
}

void FilesystemObserverTests::HandleInvalidPath()
{
    std::vector<FileEvent> receivedNotifications;

    const auto onNotifications = [&](FileEvent&& notification) {
        receivedNotifications.emplace_back(std::move(notification));
    };

    FileSystemObserver observer{ std::make_unique<FileSystemMonitor>(), "" };
    observer.StartMonitoring(onNotifications);

    QVERIFY(observer.IsActive() == false);
}

REGISTER_TEST(FilesystemObserverTests);
