#include "controllerTests.h"

#include "Mocks/mockView.h"
#include "testUtilities.hpp"

#include <Monitor/fileMonitorBase.h>
#include <Visualizations/squarifiedTreemap.h>
#include <controller.h>

#include <filesystem>
#include <memory>

namespace
{
    ControllerParameters SetupControllerParameters(std::shared_ptr<MockView>& view)
    {
        ControllerParameters parameters;

        parameters.createView = [&](Controller& controller) {
            view = std::make_shared<MockView>(controller);
            return view;
        };

        parameters.createModel = [](std::unique_ptr<FileMonitorBase> fileMonitor,
                                    const std::filesystem::path& path) {
            return std::make_shared<SquarifiedTreeMap>(std::move(fileMonitor), path);
        };

        return parameters;
    }

    std::shared_ptr<BaseTaskbarButton> GetTaskbarButton()
    {
#if defined(Q_OS_WIN)
        return std::make_shared<WinTaskbarButton>(nullptr);
#elif defined(Q_OS_LINUX)
        return std::make_shared<UnixTaskbarButton>(nullptr);
#endif // Q_OS_LINUX
    }

    std::filesystem::path GetSampleDirectory()
    {
        return TestUtilities::SanitizePath(std::filesystem::absolute("../../Tests/Sandbox"));
    }
} // namespace

void ControllerTests::initTestCase()
{
    TestUtilities::UnzipTestData(
        std::filesystem::absolute("../../Tests/Data/boost-asio.zip"),
        std::filesystem::absolute("../../Tests/Sandbox"));
}

void ControllerTests::cleanupTestCase()
{
    std::filesystem::remove_all("../../Tests/Sandbox");
}

void ControllerTests::init()
{
}

void ControllerTests::LaunchMainWindow() const
{
    std::shared_ptr<MockView> view;

    Controller controller{ SetupControllerParameters(view) };

    REQUIRE_CALL(*view, Show()).TIMES(1);

    controller.LaunchUI();
}

void ControllerTests::ScanDrive() const
{
    std::shared_ptr<MockView> view;

    Controller controller{ SetupControllerParameters(view) };
    controller.GetPersistentSettings().MonitorFileSystem(false);

    REQUIRE_CALL(*view, AskUserToLimitFileSize(trompeloeil::_, trompeloeil::_)).RETURN(true);
    REQUIRE_CALL(*view, SetWaitCursor());
    REQUIRE_CALL(*view, RestoreDefaultCursor());
    REQUIRE_CALL(*view, GetWindowHandle()).RETURN(nullptr);
    REQUIRE_CALL(*view, OnScanStarted()).TIMES(1);
    REQUIRE_CALL(*view, OnScanCompleted()).TIMES(1);
    REQUIRE_CALL(*view, GetTaskbarButton()).TIMES(1).RETURN(GetTaskbarButton());
    REQUIRE_CALL(*view, SetStatusBarMessage(trompeloeil::_, trompeloeil::_))
        .WITH(_1.find(L"Files Scanned") != std::wstring::npos)
        .TIMES(AT_LEAST(1));

    FORBID_CALL(*view, DisplayErrorDialog(trompeloeil::_));

    Settings::VisualizationParameters parameters;
    parameters.forceNewScan = true;
    parameters.rootDirectory = GetSampleDirectory().wstring();
    parameters.minimumFileSize = 0;
    parameters.onlyShowDirectories = false;

    QSignalSpy completionSpy{ &controller, &Controller::FinishedScanning };
    controller.ScanDrive(parameters);
    completionSpy.wait(10'000);
}

REGISTER_TEST(ControllerTests)
