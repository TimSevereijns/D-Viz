#include "controllerTests.h"

#include "Mocks/mockView.h"
#include "testUtilities.hpp"

#include <Factories/modelFactoryInterface.h>
#include <Factories/viewFactoryInterface.h>
#include <Monitor/fileMonitorBase.h>
#include <Visualizations/squarifiedTreemap.h>
#include <controller.h>

#include <filesystem>
#include <memory>

namespace
{
    class TestViewFactory final : public ViewFactoryInterface
    {
      public:
        TestViewFactory(std::shared_ptr<MockView>& view) : m_view{ view }
        {
        }

        auto CreateView(Controller& controller) const -> std::shared_ptr<BaseView> override
        {
            m_view = std::make_shared<MockView>(controller);
            return m_view;
        }

      private:
        std::shared_ptr<MockView>& m_view;
    };

    class TestModelFactory final : public ModelFactoryInterface
    {
      public:
        auto CreateModel(
            std::unique_ptr<FileMonitorBase> fileMonitor, const std::filesystem::path& path) const
            -> std::shared_ptr<BaseModel> override
        {
            return std::make_shared<SquarifiedTreeMap>(std::move(fileMonitor), path);
        }
    };

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

    auto viewFactory = TestViewFactory{ view };
    auto modelFactory = TestModelFactory{};

    Controller controller{ viewFactory, modelFactory };

    REQUIRE_CALL(*view, Show()).TIMES(1);

    controller.LaunchUI();
}

void ControllerTests::ScanDrive() const
{
    std::shared_ptr<MockView> view;

    auto viewFactory = TestViewFactory{ view };
    auto modelFactory = TestModelFactory{};

    Controller controller{ viewFactory, modelFactory };
    controller.GetPersistentSettings().MonitorFileSystem(false);

    REQUIRE_CALL(*view, SetWaitCursor()).TIMES(1);
    REQUIRE_CALL(*view, RestoreDefaultCursor()).TIMES(1);
    REQUIRE_CALL(*view, GetWindowHandle()).RETURN(nullptr);
    REQUIRE_CALL(*view, OnScanStarted()).TIMES(1);
    REQUIRE_CALL(*view, OnScanCompleted()).TIMES(1);
    REQUIRE_CALL(*view, GetTaskbarButton()).TIMES(1).RETURN(GetTaskbarButton());

    REQUIRE_CALL(*view, AskUserToLimitFileSize(trompeloeil::_, trompeloeil::_))
        .TIMES(1)
        .RETURN(true);

    REQUIRE_CALL(*view, SetStatusBarMessage(trompeloeil::_, trompeloeil::_))
        .WITH(_1.find(L"Files Scanned") != std::wstring::npos && _2 == 0)
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
