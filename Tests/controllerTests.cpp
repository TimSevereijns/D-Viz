#include "controllerTests.h"

#include "Utilities/testUtilities.hpp"

#include <controller.h>

#include <filesystem>
#include <memory>

namespace
{
    class FakeTaskbarButton : public BaseTaskbarButton
    {
    };

    std::shared_ptr<BaseTaskbarButton> GetFakeTaskbarButton()
    {
        return std::make_shared<FakeTaskbarButton>();
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
    m_viewFactory = std::make_shared<TestViewFactory>(m_view);
    m_modelFactory = std::make_shared<TestModelFactory>();

    m_controller = std::make_shared<Controller>(*m_viewFactory, *m_modelFactory);
    m_controller->GetPersistentSettings().MonitorFileSystem(false);
}

void ControllerTests::LaunchMainWindow() const
{
    REQUIRE_CALL(*m_view, Show()).TIMES(1);
    m_controller->LaunchUI();
}

void ControllerTests::ScanDrive() const
{
    REQUIRE_CALL(*m_view, SetWaitCursor()).TIMES(1);
    REQUIRE_CALL(*m_view, RestoreDefaultCursor()).TIMES(1);
    REQUIRE_CALL(*m_view, GetWindowHandle()).RETURN(nullptr);
    REQUIRE_CALL(*m_view, OnScanStarted()).TIMES(1);
    REQUIRE_CALL(*m_view, OnScanCompleted()).TIMES(1);
    REQUIRE_CALL(*m_view, GetTaskbarButton()).TIMES(1).RETURN(GetFakeTaskbarButton());

    REQUIRE_CALL(*m_view, AskUserToLimitFileSize(trompeloeil::_, trompeloeil::_))
        .TIMES(1)
        .RETURN(true);

    REQUIRE_CALL(*m_view, SetStatusBarMessage(trompeloeil::_, trompeloeil::_))
        .WITH(_1.find(L"Files Scanned") != std::wstring::npos && _2 == 0)
        .TIMES(AT_LEAST(1));

    FORBID_CALL(*m_view, DisplayErrorDialog(trompeloeil::_));

    Settings::VisualizationParameters parameters;
    parameters.forceNewScan = true;
    parameters.rootDirectory = GetSampleDirectory().wstring();
    parameters.minimumFileSize = 0;
    parameters.onlyShowDirectories = false;

    QSignalSpy completionSpy{ m_controller.get(), &Controller::FinishedScanning };
    m_controller->ScanDrive(parameters);
    completionSpy.wait(10'000);
}

void ControllerTests::HasModelBeenLoaded() const
{
    ScanDrive();

    QCOMPARE(m_controller->HasModelBeenLoaded(), true);
}

void ControllerTests::SelectingANode()
{
    ScanDrive();

    QCOMPARE(m_controller->GetSelectedNode(), nullptr);

    const auto* targetNode = m_controller->GetTree().GetRoot()->GetFirstChild();
    const auto callback = [&](const Tree<VizBlock>::Node& selectedNode) {
        QCOMPARE(&selectedNode, targetNode);
    };

    m_controller->SelectNode(*targetNode, callback);
    QCOMPARE(m_controller->GetSelectedNode(), targetNode);
}

REGISTER_TEST(ControllerTests)
