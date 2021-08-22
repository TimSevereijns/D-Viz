#include "controllerTests.h"

#include "Utilities/testUtilities.h"

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

    std::string GetLargeDirectoryToScan()
    {
#if defined(Q_OS_WIN)
        std::string path;
        path.reserve(512);

        std::size_t requiredBufferSize = 0;
        getenv_s(&requiredBufferSize, &path[0], path.capacity(), "GITHUB_WORKSPACE");
        path.resize(requiredBufferSize);

        const auto variableNotFound = requiredBufferSize == 0;
        if (variableNotFound) {
            return std::filesystem::current_path().root_name().string();
        }

        return path;

#elif defined(Q_OS_LINUX)

        const auto* env = std::getenv("GITHUB_WORKSPACE");
        const std::string path =
            env ? std::string{ env }
                : std::string{ std::filesystem::current_path().root_name().string() };

        return path;

#endif
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
    REQUIRE_CALL(*m_view, AskUserToLimitFileSize(trompeloeil::_)).TIMES(1).RETURN(true);

    REQUIRE_CALL(*m_view, SetStatusBarMessage(trompeloeil::_, trompeloeil::_))
        .WITH(_1.find("Files Scanned") != std::string::npos && _2 == 0)
        .TIMES(AT_LEAST(1));

    FORBID_CALL(*m_view, DisplayErrorDialog(trompeloeil::_));

    Settings::VisualizationParameters parameters;
    parameters.forceNewScan = true;
    parameters.rootDirectory = GetSampleDirectory().string();
    parameters.minimumFileSize = 0;
    parameters.onlyShowDirectories = false;

    QSignalSpy completionSpy{ m_controller.get(), &Controller::FinishedScanning };
    m_controller->ScanDrive(parameters);
    completionSpy.wait(10'000);
}

void ControllerTests::ScanDriveWithEmptyPath() const
{
    FORBID_CALL(*m_view, SetWaitCursor());
    FORBID_CALL(*m_view, RestoreDefaultCursor());
    FORBID_CALL(*m_view, GetWindowHandle());
    FORBID_CALL(*m_view, OnScanStarted());
    FORBID_CALL(*m_view, OnScanCompleted());
    FORBID_CALL(*m_view, GetTaskbarButton());
    FORBID_CALL(*m_view, AskUserToLimitFileSize(trompeloeil::_));
    FORBID_CALL(*m_view, SetStatusBarMessage(trompeloeil::_, trompeloeil::_));
    FORBID_CALL(*m_view, DisplayErrorDialog(trompeloeil::_));

    Settings::VisualizationParameters parameters;
    parameters.forceNewScan = true;
    parameters.rootDirectory = "";
    parameters.minimumFileSize = 0;
    parameters.onlyShowDirectories = false;

    m_controller->ScanDrive(parameters); //< Should return immediately.
}

void ControllerTests::CancelScan() const
{
    QVERIFY(m_controller);

    // We need a larger directory so that we have a bit more time to cancel the scan.
    const auto path = GetLargeDirectoryToScan();

    Settings::VisualizationParameters parameters;
    parameters.forceNewScan = true;
    parameters.rootDirectory = path;
    parameters.minimumFileSize = 0;
    parameters.onlyShowDirectories = false;

    REQUIRE_CALL(*m_view, SetWaitCursor()).TIMES(1);
    REQUIRE_CALL(*m_view, RestoreDefaultCursor()).TIMES(1);
    REQUIRE_CALL(*m_view, GetWindowHandle()).RETURN(nullptr);
    REQUIRE_CALL(*m_view, OnScanStarted()).TIMES(1);
    REQUIRE_CALL(*m_view, OnScanCompleted()).TIMES(1);
    REQUIRE_CALL(*m_view, GetTaskbarButton()).TIMES(1).RETURN(GetFakeTaskbarButton());
    REQUIRE_CALL(*m_view, AskUserToLimitFileSize(trompeloeil::_)).TIMES(1).RETURN(true);

    std::vector<std::string> messages;

    constexpr auto& messageFragment = "Files Scanned: ";

    REQUIRE_CALL(*m_view, SetStatusBarMessage(trompeloeil::_, trompeloeil::_))
        .WITH(_1.find(messageFragment) != std::string::npos && _2 == 0)
        .LR_SIDE_EFFECT(messages.emplace_back(std::move(_1)))
        .TIMES(AT_LEAST(1));

    QSignalSpy completionSpy{ m_controller.get(), &Controller::FinishedScanning };
    m_controller->ScanDrive(parameters);

    // Brief pause to make sure the scanning thread gets instantiated.
    std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });

    m_controller->StopScanning();
    completionSpy.wait(10'000);

    QVERIFY(messages.size() > 0);

    const auto& message = messages.front();

    const auto lhs = message.find(messageFragment);
    const auto rhs = message.find(" ", lhs + std::size(messageFragment));

    const auto filesScanned = message.substr(
        lhs + std::size(messageFragment) - 1, rhs - (lhs + std::size(messageFragment) - 1));

    const int count = std::count_if(
        std::filesystem::recursive_directory_iterator{ path },
        std::filesystem::recursive_directory_iterator{}, [](const auto&) { return true; });

    QVERIFY(std::stoi(filesScanned) < count);
}

void ControllerTests::HasModelBeenLoaded() const
{
    ScanDrive();

    QCOMPARE(m_controller->HasModelBeenLoaded(), true);
}

void ControllerTests::SelectNode()
{
    QVERIFY(m_controller);

    ScanDrive();

    QCOMPARE(m_controller->GetSelectedNode(), nullptr);

    const auto* targetNode = m_controller->GetTree().GetRoot()->GetFirstChild();
    QVERIFY(targetNode);

    const auto callback = [&](const Tree<VizBlock>::Node& selectedNode) {
        QCOMPARE(&selectedNode, targetNode);
    };

    m_controller->SelectNode(*targetNode, callback);
    QCOMPARE(m_controller->GetSelectedNode(), targetNode);
}

void ControllerTests::ClearSelectedNode()
{
    SelectNode();

    m_controller->ClearSelectedNode();
    QCOMPARE(m_controller->GetSelectedNode(), nullptr);
}

void ControllerTests::VerifyFilesOverLimitAreDisplayed() const
{
    QVERIFY(m_controller);
    using namespace Literals::Numeric::Binary;

    Settings::VisualizationParameters parameters;
    parameters.minimumFileSize = 1_KiB;
    parameters.onlyShowDirectories = false;

    VizBlock sample;
    sample.file.name = "Foo";
    sample.file.extension = ".txt";
    sample.file.size = 16_KiB;
    sample.file.type = FileType::Regular;

    QCOMPARE(parameters.IsNodeVisible(sample), true);
}

void ControllerTests::VerifyFilesUnderLimitAreNotDisplayed() const
{
    QVERIFY(m_controller);
    using namespace Literals::Numeric::Binary;

    Settings::VisualizationParameters parameters;
    parameters.minimumFileSize = 32_KiB;
    parameters.onlyShowDirectories = false;

    VizBlock sample;
    sample.file.name = "Foo";
    sample.file.extension = ".txt";
    sample.file.size = 16_KiB;
    sample.file.type = FileType::Regular;

    QCOMPARE(parameters.IsNodeVisible(sample), false);
}

void ControllerTests::VerifyFilesAreNotDisplayedWhenOnlyDirectoriesAllowed() const
{
    QVERIFY(m_controller);
    using namespace Literals::Numeric::Binary;

    Settings::VisualizationParameters parameters;
    parameters.minimumFileSize = 10_MiB;
    parameters.onlyShowDirectories = true;

    VizBlock sample;
    sample.file.name = "Bar";
    sample.file.extension = "";
    sample.file.size = 10_GiB;
    sample.file.type = FileType::Regular;

    QCOMPARE(parameters.IsNodeVisible(sample), false);
}

void ControllerTests::VerifyDirectoriesUnderLimitAreNotShownWhenNotAllowed() const
{
    QVERIFY(m_controller);
    using namespace Literals::Numeric::Binary;

    Settings::VisualizationParameters parameters;
    parameters.minimumFileSize = 1_MiB;
    parameters.onlyShowDirectories = true;

    VizBlock sample;
    sample.file.name = "Bar";
    sample.file.extension = "";
    sample.file.size = 10_MiB;
    sample.file.type = FileType::Directory;

    QCOMPARE(parameters.IsNodeVisible(sample), true);
}

void ControllerTests::VerifyDirectoriesOverLimitAreNotShownWhenNotAllowed() const
{
    QVERIFY(m_controller);
    using namespace Literals::Numeric::Binary;

    Settings::VisualizationParameters parameters;
    parameters.minimumFileSize = 10_MiB;
    parameters.onlyShowDirectories = true;

    VizBlock sample;
    sample.file.name = "Bar";
    sample.file.extension = "";
    sample.file.size = 1_MiB;
    sample.file.type = FileType::Directory;

    QCOMPARE(parameters.IsNodeVisible(sample), false);
}

void ControllerTests::SearchTreemapWithoutPriorSelection() const
{
    QVERIFY(m_controller);

    constexpr auto query = ".hpp";

    const auto deselectionCallback = [](const std::vector<const Tree<VizBlock>::Node*>& nodes) {
        QCOMPARE(nodes.empty(), true);
    };

    const auto selectionCallback = [&](const std::vector<const Tree<VizBlock>::Node*>& nodes) {
        QCOMPARE(nodes.empty(), false);
        QVERIFY(std::all_of(std::begin(nodes), std::end(nodes), [&](const auto* node) {
            return node->GetData().file.extension == query;
        }));
    };

    REQUIRE_CALL(*m_view, SetStatusBarMessage(trompeloeil::_, trompeloeil::_)).TIMES(1);

    ScanDrive();

    m_controller->SearchTreeMap(
        query, deselectionCallback, selectionCallback, SearchFlags::SearchFiles);
}

void ControllerTests::SearchTreemapWithPriorSelection() const
{
    QVERIFY(m_controller);

    constexpr auto query = ".hpp";
    constexpr auto prior = ".ipp";

    const auto deselectionCallback = [&](const std::vector<const Tree<VizBlock>::Node*>& nodes) {
        // We'll expect the previous highlighted nodes to be deselected prior to the highlighting
        // of the search results.

        QCOMPARE(nodes.empty(), false);
        QVERIFY(std::all_of(std::begin(nodes), std::end(nodes), [&](const auto* node) {
            return node->GetData().file.extension == prior;
        }));
    };

    const auto selectionCallback = [&](const std::vector<const Tree<VizBlock>::Node*>& nodes) {
        QCOMPARE(nodes.empty(), false);
        QVERIFY(std::all_of(std::begin(nodes), std::end(nodes), [&](const auto* node) {
            return node->GetData().file.extension == query;
        }));
    };

    const auto highlightCallback = [&](const std::vector<const Tree<VizBlock>::Node*>& nodes) {
        QCOMPARE(nodes.empty(), false);
        QVERIFY(std::all_of(std::begin(nodes), std::end(nodes), [&](const auto* node) {
            return node->GetData().file.extension == prior;
        }));
    };

    REQUIRE_CALL(*m_view, SetStatusBarMessage(trompeloeil::_, trompeloeil::_)).TIMES(2);

    ScanDrive();

    m_controller->HighlightAllMatchingExtensions(prior, highlightCallback);

    m_controller->SearchTreeMap(
        query, deselectionCallback, selectionCallback, SearchFlags::SearchFiles);
}

void ControllerTests::SearchTreemapWithIncorrectFlags() const
{
    ScanDrive();

    const auto callback = [](const std::vector<const Tree<VizBlock>::Node*>&) { QVERIFY(false); };

    // Since we're not passing either the SearchFiles or SearchDirectory flags, the search function
    // should hit an early return and no callbacks should be invoked.
    m_controller->SearchTreeMap("socket", callback, callback, static_cast<SearchFlags>(0));
}

void ControllerTests::HighlightAncestors() const
{
    ScanDrive();

    const auto selectionCallback = [&](const std::vector<const Tree<VizBlock>::Node*>& nodes) {
        QCOMPARE(nodes.size(), 3ul);
    };

    REQUIRE_CALL(*m_view, SetStatusBarMessage(trompeloeil::_, trompeloeil::_)).TIMES(1);

    const auto* firstChild = m_controller->GetTree().GetRoot()->GetFirstChild();
    QVERIFY(firstChild != nullptr);

    const auto* firstGrandchild = firstChild->GetFirstChild();
    QVERIFY(firstGrandchild != nullptr);

    const auto* firstGreatGrandchild = firstGrandchild->GetFirstChild();
    QVERIFY(firstGreatGrandchild != nullptr);

    m_controller->HighlightAncestors(*firstGreatGrandchild, selectionCallback);
}

void ControllerTests::IsNodeHighlighted() const
{
    ScanDrive();

    const auto selectionCallback = [&](const std::vector<const Tree<VizBlock>::Node*>& nodes) {
        QCOMPARE(nodes.size(), m_controller->GetHighlightedNodes().size());
    };

    REQUIRE_CALL(*m_view, SetStatusBarMessage(trompeloeil::_, trompeloeil::_)).TIMES(1);

    const auto* firstChild = m_controller->GetTree().GetRoot()->GetFirstChild();
    QVERIFY(firstChild != nullptr);

    m_controller->HighlightAncestors(*firstChild, selectionCallback);
    m_controller->IsNodeHighlighted(*firstChild);
}

void ControllerTests::HighlightDescendants() const
{
    ScanDrive();

    const auto selectionCallback = [&](const std::vector<const Tree<VizBlock>::Node*>& nodes) {
        QCOMPARE(nodes.size(), 469ul); //< As seen in File Explorer
    };

    REQUIRE_CALL(*m_view, SetStatusBarMessage(trompeloeil::_, trompeloeil::_)).TIMES(1);

    const auto* rootNode = m_controller->GetTree().GetRoot();
    QVERIFY(rootNode != nullptr);

    m_controller->HighlightDescendants(*rootNode, selectionCallback);
}

void ControllerTests::SelectNodeViaRay() const
{
    ScanDrive();

    const std::string targetName = "socket_ops.ipp";

    const auto targetNode = std::find_if(
        Tree<VizBlock>::LeafIterator{ m_controller->GetTree().GetRoot() },
        Tree<VizBlock>::LeafIterator{},
        [&](const auto& node) { return (node->file.name + node->file.extension) == targetName; });

    QVERIFY(targetNode != Tree<VizBlock>::LeafIterator{});

    const auto targetBlock = targetNode->GetData().block;
    const auto x = static_cast<float>(targetBlock.GetOrigin().x() + targetBlock.GetWidth() / 2.0);
    const auto y = static_cast<float>(targetBlock.GetOrigin().y() + targetBlock.GetHeight());
    const auto z = static_cast<float>(targetBlock.GetOrigin().z() - targetBlock.GetDepth() / 2.0);

    Camera camera;
    camera.SetPosition({ 300, 300, -300 });
    camera.LookAt({ x, y, z });

    const Ray ray{ camera.GetPosition(), camera.Forward() };

    const auto deselectionCallback = [](const Tree<VizBlock>::Node&) { QVERIFY(false); };

    const auto selectionCallback = [&](const Tree<VizBlock>::Node& node) {
        QCOMPARE(node->file.name, "socket_ops");
        QCOMPARE(node->file.extension, ".ipp");
    };

    REQUIRE_CALL(*m_view, SetStatusBarMessage(trompeloeil::_, trompeloeil::_)).TIMES(1);

    m_controller->SelectNodeViaRay(camera, ray, deselectionCallback, selectionCallback);
}

void ControllerTests::ConsecutiveNodeSelection() const
{
    ScanDrive();

    const std::vector<std::vector<float>> targets = {
        { 135, 10, -60 }, //< "socket_ops.ipp"
        { 535, 10, -60 }  //< "socket_types.hpp"
    };

    std::vector<std::string> selectionsMade;
    std::vector<std::string> deselectionsMade;

    REQUIRE_CALL(*m_view, SetStatusBarMessage(trompeloeil::_, trompeloeil::_)).TIMES(2);

    for (const auto& target : targets) {
        Camera camera;
        camera.SetPosition({ 300, 300, -300 });
        camera.LookAt({ target[0], target[1], target[2] });

        const Ray ray{ camera.GetPosition(), camera.Forward() };

        const auto deselectionCallback = [&](const Tree<VizBlock>::Node& node) {
            deselectionsMade.emplace_back(node->file.name + node->file.extension);
        };

        const auto selectionCallback = [&](const Tree<VizBlock>::Node& node) {
            selectionsMade.emplace_back(node->file.name + node->file.extension);
        };

        m_controller->SelectNodeViaRay(camera, ray, deselectionCallback, selectionCallback);
    }

    QCOMPARE(selectionsMade.size(), 2ul);   //< "socket_ops.ipp" and "socket_types.hpp"
    QCOMPARE(deselectionsMade.size(), 1ul); //< "socket_ops.ipp"

    QCOMPARE(selectionsMade.front(), deselectionsMade.front());
}

void ControllerTests::SelectEmptyAir() const
{
    ScanDrive();

    REQUIRE_CALL(*m_view, SetStatusBarMessage(trompeloeil::_, trompeloeil::_))
        .WITH(_1 == "Scanned 469 files and 21 directories." && _2 == 0)
        .TIMES(1);

    Camera camera;
    camera.SetPosition({ 300, 300, -300 });
    camera.LookAt({ 135, 300, -60 });

    const Ray ray{ camera.GetPosition(), camera.Forward() };

    const auto callback = [](const Tree<VizBlock>::Node&) { QVERIFY(false); };
    m_controller->SelectNodeViaRay(camera, ray, callback, callback);
}

void ControllerTests::SelectNodeViaRayBeforeModelLoads() const
{
    FORBID_CALL(*m_view, SetStatusBarMessage(trompeloeil::_, trompeloeil::_));

    Camera camera;
    camera.SetPosition({ 300, 300, -300 });
    camera.LookAt({ 1.0f, 1.0f, 1.0f });

    const Ray ray{ camera.GetPosition(), camera.Forward() };

    const auto callback = [](const Tree<VizBlock>::Node&) { QVERIFY(false); };
    m_controller->SelectNodeViaRay(camera, ray, callback, callback);
}

void ControllerTests::DetermineDefaultLeafNodeColor() const
{
    ScanDrive();

    const std::string targetName = "async_result.hpp";

    const auto targetNode = std::find_if(
        Tree<VizBlock>::LeafIterator{ m_controller->GetTree().GetRoot() },
        Tree<VizBlock>::LeafIterator{},
        [&](const auto& node) { return (node->file.name + node->file.extension) == targetName; });

    const auto& nodeColor = m_controller->DetermineNodeColor(*targetNode);
    QCOMPARE(nodeColor, Constants::Colors::File);
}

void ControllerTests::DetermineDefaultColorOfHighlightedNode() const
{
    SearchTreemapWithoutPriorSelection();

    const std::string targetName = "async_result.hpp";

    const auto targetNode = std::find_if(
        Tree<VizBlock>::LeafIterator{ m_controller->GetTree().GetRoot() },
        Tree<VizBlock>::LeafIterator{},
        [&](const auto& node) { return (node->file.name + node->file.extension) == targetName; });

    const auto& nodeColor = m_controller->DetermineNodeColor(*targetNode);
    QCOMPARE(nodeColor, Constants::Colors::Highlighted);
}

void ControllerTests::DetermineCustomColorOfRegisteredNode() const
{
    ScanDrive();

    const std::string targetName = "async_result.hpp";

    const auto targetNode = std::find_if(
        Tree<VizBlock>::LeafIterator{ m_controller->GetTree().GetRoot() },
        Tree<VizBlock>::LeafIterator{},
        [&](const auto& node) { return (node->file.name + node->file.extension) == targetName; });

    constexpr auto customColor = QVector3D{ 0.1f, 0.2f, 0.3f };
    m_controller->RegisterNodeColor(*targetNode, customColor);
    const auto& nodeColor = m_controller->DetermineNodeColor(*targetNode);

    QCOMPARE(nodeColor, customColor);
}

void ControllerTests::PrintingMetadataToStatusBar() const
{
    REQUIRE_CALL(*m_view, SetStatusBarMessage(trompeloeil::_, trompeloeil::_))
        .WITH(_1.find("Scanned 469 files and 21 directories.") != std::string::npos && _2 == 0)
        .TIMES(1);

    ScanDrive();

    m_controller->PrintMetadataToStatusBar();
}

void ControllerTests::GetRootPath() const
{
    ScanDrive();

    const auto rootPath = m_controller->GetRootPath();
    const auto rootFileName = m_controller->GetTree().GetRoot()->GetData().file.name;

    QCOMPARE(rootPath, rootFileName);
}

REGISTER_TEST(ControllerTests)
