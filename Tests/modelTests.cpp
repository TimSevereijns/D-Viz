#include "modelTests.h"

#include <Model/Scanner/scanningParameters.h>
#include <Model/Scanner/scanningProgress.h>
#include <Utilities/operatingSystem.h>
#include <constants.h>

#include "Utilities/testUtilities.h"

namespace
{
    std::filesystem::path PathToNode(const Tree<VizBlock>::Node& node)
    {
        std::vector<std::reference_wrapper<const std::string>> reversePath;
        reversePath.reserve(Tree<VizBlock>::Depth(node));
        reversePath.emplace_back(node->file.name);

        const auto* currentNode = &node;
        while (currentNode->GetParent() && currentNode->GetParent()->GetParent()) {
            currentNode = currentNode->GetParent();
            reversePath.emplace_back(currentNode->GetData().file.name);
        }

        const auto pathFromRoot = std::accumulate(
            std::rbegin(reversePath), std::rend(reversePath), std::string{},
            [](const std::string& path, const std::string& file) {
                constexpr auto slash = '/';

                if (!path.empty() && path.back() != slash) {
                    return path + slash + file;
                }

                return path + file;
            });

        auto finalPath = std::filesystem::path{ pathFromRoot };
        finalPath.make_preferred();

        return finalPath;
    }

    std::vector<FileEvent> SelectAllFiles(
        const typename Tree<VizBlock>::Node& rootNode, const std::string_view& fileExtension,
        FileEventType eventType)
    {
        std::vector<FileEvent> allEvents;

        std::for_each(
            Tree<VizBlock>::LeafIterator{ &rootNode }, Tree<VizBlock>::LeafIterator{},
            [&](const auto& node) {
                if (node->file.extension != fileExtension) {
                    return;
                }

                const auto path = PathToNode(node);
                allEvents.emplace_back(
                    FileEvent{ path.string() + node->file.extension, eventType });
            });

        return allEvents;
    }

} // namespace

ModelTests::ModelTests()
{
    m_sampleDirectory = TestUtilities::SanitizePath(m_sampleDirectory);
}

void ModelTests::initTestCase()
{
    TestUtilities::UnzipTestData(
        std::filesystem::absolute("../../Tests/Data/boost-asio.zip"),
        std::filesystem::absolute("../../Tests/Sandbox"));

    const auto progressCallback = [&](const ScanningProgress& /*progress*/) {
        ++m_progressCallbackInvocations;
    };

    const auto completionCallback = [&](const ScanningProgress& progress,
                                        std::shared_ptr<Tree<VizBlock>> tree) {
        QVERIFY(tree != nullptr);

        m_bytesScanned = progress.bytesProcessed.load();
        m_filesScanned = progress.filesScanned.load();
        m_directoriesScanned = progress.directoriesScanned.load();

        m_tree = std::move(tree);
    };

    QSignalSpy completionSpy{ &m_scanner, &DriveScanner::Finished };

    const ScanningParameters parameters{ m_sampleDirectory, progressCallback, completionCallback };
    m_scanner.StartScanning(parameters);

    completionSpy.wait(10'000);
}

void ModelTests::cleanupTestCase()
{
    std::filesystem::remove_all("../../Tests/Sandbox");
}

void ModelTests::init()
{
    QVERIFY(m_tree != nullptr);

    const auto notificationGenerator = [&]() -> std::optional<FileEvent> {
        if (m_sampleNotifications.empty()) {
            return std::nullopt;
        }

        auto nextNotification = m_sampleNotifications.back();
        m_sampleNotifications.pop_back();

        return nextNotification;
    };

    m_model = std::make_unique<SquarifiedTreeMap>(
        std::make_unique<MockFileMonitor>(notificationGenerator), m_sampleDirectory);

    m_model->Parse(m_tree);
}

void ModelTests::ProgressCallbackIsInvoked()
{
    QVERIFY(m_progressCallbackInvocations > 0); //< Scanning time determines exact count.
}

void ModelTests::ModelIsPopulated()
{
    const auto& tree = m_model->GetTree();

    // Number of items in sample directory:
    QCOMPARE(static_cast<unsigned long>(tree.Size()), 490ul);
}

void ModelTests::ScanningProgressDataIsCorrect()
{
    // Counts as seen in Windows File Explorer:
    QCOMPARE(static_cast<unsigned long>(m_bytesScanned), 3'407'665ul);
    QCOMPARE(static_cast<unsigned long>(m_filesScanned), 469ul);
    QCOMPARE(static_cast<unsigned long>(m_directoriesScanned), 20ul);
}

void ModelTests::GetRootPath()
{
    const auto path = m_model->GetRootPath();

    const auto testDirectory = std::filesystem::absolute("../../Tests/Sandbox/asio");
    const auto expectedPath = TestUtilities::SanitizePath(testDirectory);

    QCOMPARE(expectedPath.string(), path);
}

void ModelTests::GenerateReferenceBlock()
{
    const auto referenceBlock = Block{ PrecisePoint{ 0.0, 0.0, 0.0 },
                                       /* width =  */ 1.0,
                                       /* height = */ 1.0,
                                       /* depth =  */ 1.0,
                                       /* generateVertices = */ true };

    const auto& vertices = referenceBlock.GetVerticesAndNormals();

    // Front face:
    QCOMPARE(vertices[0], QVector3D(0.0f, 0.0f, 0.0f));
    QCOMPARE(vertices[6], QVector3D(1.0f, 1.0f, 0.0f));
    QCOMPARE(vertices[10], QVector3D(1.0f, 0.0f, 0.0f));

    // Top face:
    QCOMPARE(vertices[48], QVector3D(0.0f, 1.0f, 0.0f));
    QCOMPARE(vertices[54], QVector3D(1.0f, 1.0f, -1.0f));
    QCOMPARE(vertices[58], QVector3D(1.0f, 1.0f, 0.0f));
}

void ModelTests::SelectingNodes()
{
    QVERIFY(m_model->GetSelectedNode() == nullptr);

    const Tree<VizBlock>::Node* sampleNode = m_tree->GetRoot();
    m_model->SelectNode(*sampleNode);
    QVERIFY(m_model->GetSelectedNode() == sampleNode);

    m_model->ClearSelectedNode();
    QVERIFY(m_model->GetSelectedNode() == nullptr);
}

void ModelTests::HighlightDescendants()
{
    QVERIFY(m_model->GetHighlightedNodes().size() == 0);

    Settings::VisualizationParameters parameters;
    parameters.rootDirectory = "";
    parameters.minimumFileSize = 0u;
    parameters.onlyShowDirectories = false;

    const Tree<VizBlock>::Node* rootNode = m_tree->GetRoot();
    m_model->HighlightDescendants(*rootNode, parameters);

    const auto leafCount = std::count_if(
        Tree<VizBlock>::LeafIterator{ rootNode }, Tree<VizBlock>::LeafIterator{},
        [](const auto&) { return true; });

    QCOMPARE(
        static_cast<std::int32_t>(m_model->GetHighlightedNodes().size()),
        static_cast<std::int32_t>(leafCount));
}

void ModelTests::HighlightAncestors()
{
    QVERIFY(m_model->GetHighlightedNodes().size() == 0);

    const auto target = std::find_if(
        Tree<VizBlock>::LeafIterator{ m_tree->GetRoot() }, Tree<VizBlock>::LeafIterator{},
        [](const auto& node) {
            return (node->file.name + node->file.extension) == "endpoint.ipp";
        });

    QVERIFY(target != Tree<VizBlock>::SiblingIterator{});

    m_model->HighlightAncestors(*target);

    QCOMPARE(
        static_cast<std::int32_t>(m_model->GetHighlightedNodes().size()),
        static_cast<std::int32_t>(4));
}

void ModelTests::HighlightAllMatchingFileNames()
{
    QVERIFY(m_model->GetHighlightedNodes().size() == 0);

    Settings::VisualizationParameters parameters;
    parameters.rootDirectory = "";
    parameters.minimumFileSize = 0u;
    parameters.onlyShowDirectories = false;

    constexpr auto shouldSearchFiles{ true };
    constexpr auto shouldSearchDirectories{ false };

    m_model->HighlightMatchingFileNames(
        "socket", parameters, shouldSearchFiles, shouldSearchDirectories);

    const auto headerCount = std::count_if(
        Tree<VizBlock>::PostOrderIterator{ m_tree->GetRoot() }, Tree<VizBlock>::PostOrderIterator{},
        [](const auto& node) { return node->file.name.find("socket") != std::string::npos; });

    QCOMPARE(
        static_cast<std::int32_t>(m_model->GetHighlightedNodes().size()),
        static_cast<std::int32_t>(headerCount));
}

void ModelTests::HighlightMatchingFileExtensions()
{
    QVERIFY(m_model->GetHighlightedNodes().size() == 0);

    Settings::VisualizationParameters parameters;
    parameters.rootDirectory = "";
    parameters.minimumFileSize = 0u;
    parameters.onlyShowDirectories = false;

    m_model->HighlightMatchingFileExtensions(".hpp", parameters);

    const auto headerCount = std::count_if(
        Tree<VizBlock>::PostOrderIterator{ m_tree->GetRoot() }, Tree<VizBlock>::PostOrderIterator{},
        [](const auto& node) { return node->file.extension == ".hpp"; });

    QCOMPARE(
        static_cast<std::int32_t>(m_model->GetHighlightedNodes().size()),
        static_cast<std::int32_t>(headerCount));
}

void ModelTests::ClearHighlightedNodes()
{
    QVERIFY(m_model->GetHighlightedNodes().size() == 0);

    Settings::VisualizationParameters parameters;
    parameters.rootDirectory = "";
    parameters.minimumFileSize = 0u;
    parameters.onlyShowDirectories = false;

    m_model->HighlightMatchingFileExtensions(".hpp", parameters);
    QCOMPARE(false, m_model->GetHighlightedNodes().empty());

    m_model->ClearHighlightedNodes();
    QCOMPARE(true, m_model->GetHighlightedNodes().empty());
}

void ModelTests::ComputeBoundingBoxes()
{
    m_model->UpdateBoundingBoxes();

    const auto rootNode = m_model->GetTree().GetRoot();
    const auto rootBlock = rootNode->GetData();
    const auto rootBoundingBox = rootBlock.boundingBox;

    QCOMPARE(rootBoundingBox.GetDepth(), rootBlock.block.GetDepth());
    QCOMPARE(rootBoundingBox.GetWidth(), rootBlock.block.GetWidth());

    const auto itr = std::max_element(
        Tree<VizBlock>::LeafIterator{ rootNode }, Tree<VizBlock>::LeafIterator{},
        [](const Tree<VizBlock>::Node& lhs, const Tree<VizBlock>::Node& rhs) {
            const auto leftHeight = lhs->block.GetOrigin().y() + lhs->block.GetHeight();
            const auto rightHeight = rhs->block.GetOrigin().y() + rhs->block.GetHeight();

            return leftHeight < rightHeight;
        });

    QVERIFY(itr != Tree<VizBlock>::LeafIterator{});

    const auto highestPoint =
        itr->GetData().block.GetOrigin().y() + itr->GetData().block.GetHeight();

    // The height of the root's bounding box should match the height of the tallest node:
    QCOMPARE(rootBoundingBox.GetHeight(), highestPoint);

    // The dimensions of the bounding box enclosing the node at the peak should be equal to itself.
    QCOMPARE(itr->GetData().block.GetWidth(), itr->GetData().boundingBox.GetWidth());
    QCOMPARE(itr->GetData().block.GetDepth(), itr->GetData().boundingBox.GetDepth());
    QCOMPARE(itr->GetData().block.GetHeight(), itr->GetData().boundingBox.GetHeight());
}

void ModelTests::CopyPathToClipboard()
{
    const std::string targetName = "socket_ops.ipp";

    const auto targetNode = std::find_if(
        Tree<VizBlock>::LeafIterator{ m_tree->GetRoot() }, Tree<VizBlock>::LeafIterator{},
        [&](const auto& node) { return (node->file.name + node->file.extension) == targetName; });

    const auto path = Controller::NodeToFilePath(*targetNode);
    OS::CopyPathToClipboard(path);

    QClipboard* clipboard = QApplication::clipboard();
    const auto text = clipboard->text();

    QCOMPARE(text.toStdString(), path.string());
}

void ModelTests::FindNearestNodeFromFront()
{
    const std::string targetName = "socket_ops.ipp";

    const auto targetNode = std::find_if(
        Tree<VizBlock>::LeafIterator{ m_tree->GetRoot() }, Tree<VizBlock>::LeafIterator{},
        [&](const auto& node) { return (node->file.name + node->file.extension) == targetName; });

    QVERIFY(targetNode != Tree<VizBlock>::LeafIterator{});

    const auto targetBlock = targetNode->GetData().block;
    const auto x = static_cast<float>(targetBlock.GetOrigin().x() + targetBlock.GetWidth() / 2.0);
    const auto y = static_cast<float>(targetBlock.GetOrigin().y() + targetBlock.GetHeight());
    const auto z = static_cast<float>(targetBlock.GetOrigin().z() - targetBlock.GetDepth() / 2.0);

    Camera camera;
    camera.SetPosition({ -300, 300, 300 });
    camera.LookAt({ x, y, z });

    const Ray ray{ camera.GetPosition(), camera.Forward() };

    Settings::VisualizationParameters parameters;
    parameters.minimumFileSize = 0;

    const auto* node = m_model->FindNearestIntersection(camera, ray, parameters);
    QVERIFY(node != nullptr);

    const auto fileName = node->GetData().file.name + node->GetData().file.extension;
    QCOMPARE(fileName, targetName);
}

void ModelTests::FindNearestNodeFromBack()
{
    const std::string targetName = "socket_ops.ipp";

    const auto targetNode = std::find_if(
        Tree<VizBlock>::LeafIterator{ m_tree->GetRoot() }, Tree<VizBlock>::LeafIterator{},
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

    Settings::VisualizationParameters parameters;
    parameters.minimumFileSize = 0;

    const auto* node = m_model->FindNearestIntersection(camera, ray, parameters);
    QVERIFY(node != nullptr);

    const auto fileName = node->GetData().file.name + node->GetData().file.extension;
    QCOMPARE(fileName, targetName);
}

void ModelTests::FindNearestNodeWithSizeLimitations()
{
    const std::string targetName = "socket_ops.ipp";

    const auto targetNode = std::find_if(
        Tree<VizBlock>::LeafIterator{ m_tree->GetRoot() }, Tree<VizBlock>::LeafIterator{},
        [&](const auto& node) { return (node->file.name + node->file.extension) == targetName; });

    QVERIFY(targetNode != Tree<VizBlock>::LeafIterator{});

    const auto targetBlock = targetNode->GetData().block;
    const auto x = static_cast<float>(targetBlock.GetOrigin().x() + targetBlock.GetWidth() / 2.0);
    const auto y = static_cast<float>(targetBlock.GetOrigin().y() + targetBlock.GetHeight());
    const auto z = static_cast<float>(targetBlock.GetOrigin().z() - targetBlock.GetDepth() / 2.0);

    Camera camera;
    camera.SetPosition({ -300, 300, 300 });
    camera.LookAt({ x, y, z });

    const Ray ray{ camera.GetPosition(), camera.Forward() };

    using namespace Literals::Numeric::Binary;

    Settings::VisualizationParameters parameters;
    parameters.minimumFileSize = 128_KiB;

    const auto* node = m_model->FindNearestIntersection(camera, ray, parameters);
    QVERIFY(node != nullptr);

    const auto fileName = node->GetData().file.name + node->GetData().file.extension;
    QCOMPARE(targetNode->GetParent()->GetData().file.name, fileName);
}

void ModelTests::ToggleFileMonitoring()
{
    m_sampleNotifications = std::vector<FileEvent>{ { "spawn.hpp", FileEventType::Touched } };

    QCOMPARE(m_model->IsFileSystemBeingMonitored(), false);
    m_model->StartMonitoringFileSystem();
    QCOMPARE(m_model->IsFileSystemBeingMonitored(), true);
    m_model->StopMonitoringFileSystem();
    QCOMPARE(m_model->IsFileSystemBeingMonitored(), false);
}

void ModelTests::TestSingleNotification(FileEventType eventType)
{
    QVERIFY(m_tree != nullptr);

    std::filesystem::path absolutePathToRoot = m_model->GetTree().GetRoot()->GetData().file.name;
    std::filesystem::path targetFile = absolutePathToRoot / "spawn.hpp";

    m_sampleNotifications = std::vector<FileEvent>{ { targetFile, eventType } };

    m_model->StartMonitoringFileSystem();
    m_model->WaitForNextModelChange();
    m_model->StopMonitoringFileSystem();

    const auto possibleNotification = m_model->FetchNextModelChange();
    QVERIFY(possibleNotification.has_value());

    QCOMPARE(possibleNotification->path, targetFile);
    QCOMPARE(possibleNotification->eventType, eventType);
}

void ModelTests::TrackSingleFileModification()
{
    TestSingleNotification(FileEventType::Touched);
}

void ModelTests::TrackSingleFileDeletion()
{
    TestSingleNotification(FileEventType::Deleted);
}

void ModelTests::TrackSingleFileRename()
{
    TestSingleNotification(FileEventType::Renamed);
}

void ModelTests::TrackMultipleDeletions()
{
    QVERIFY(m_tree != nullptr);

    m_sampleNotifications = SelectAllFiles(*m_tree->GetRoot(), ".ipp", FileEventType::Deleted);

    m_model->StartMonitoringFileSystem();

    const auto totalNotifications = m_sampleNotifications.size();
    auto processedNotifications{ 0u };

    const auto startTime = std::chrono::steady_clock::now();

    while (processedNotifications != totalNotifications) {
        std::optional<FileEvent> notification = m_model->FetchNextModelChange();

        if (notification) {
            ++processedNotifications;

            QCOMPARE(notification->eventType, FileEventType::Deleted);
            QCOMPARE(notification->path.extension(), ".ipp");
        }

        const auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startTime);

        if (elapsedTime > std::chrono::milliseconds{ 500 }) {
            break;
        }
    }

    m_model->StopMonitoringFileSystem();

    QCOMPARE(processedNotifications, totalNotifications);
}

void ModelTests::ApplyFileDeletion()
{
    std::filesystem::path absolutePathToRoot = m_model->GetTree().GetRoot()->GetData().file.name;
    std::filesystem::path targetFile = absolutePathToRoot / "basic_socket.hpp";

    m_sampleNotifications =
        std::vector<FileEvent>{ { targetFile.string(), FileEventType::Deleted } };

    const auto foundTargetNode = std::any_of(
        Tree<VizBlock>::PostOrderIterator{ m_model->GetTree().GetRoot() },
        Tree<VizBlock>::PostOrderIterator{},
        [&](const auto& node) { return node->file.name == "basic_socket"; });

    QVERIFY(foundTargetNode == true);

    m_model->StartMonitoringFileSystem();
    m_model->WaitForNextModelChange();
    m_model->RefreshTreemap();
    m_model->StopMonitoringFileSystem();

    const auto wasTargetNodeRemoved = std::none_of(
        Tree<VizBlock>::PostOrderIterator{ m_model->GetTree().GetRoot() },
        Tree<VizBlock>::PostOrderIterator{},
        [&](const auto& node) { return node->file.name == "basic_socket"; });

    QVERIFY(wasTargetNodeRemoved == true);
}

void ModelTests::ApplyFileCreation()
{
    std::filesystem::path absolutePathToRoot = m_model->GetTree().GetRoot()->GetData().file.name;
    std::filesystem::path targetFile = absolutePathToRoot / "fake_file.hpp";

    m_sampleNotifications = std::vector<FileEvent>{ { targetFile, FileEventType::Created } };

    const auto nodeDoesNotExist = std::none_of(
        Tree<VizBlock>::PostOrderIterator{ m_model->GetTree().GetRoot() },
        Tree<VizBlock>::PostOrderIterator{},
        [&](const auto& node) { return node->file.name == "fake_file"; });

    QVERIFY(nodeDoesNotExist == true);

    m_model->StartMonitoringFileSystem();
    m_model->WaitForNextModelChange();
    m_model->RefreshTreemap();
    m_model->StopMonitoringFileSystem();

    const auto nodeWasAdded = std::any_of(
        Tree<VizBlock>::PostOrderIterator{ m_model->GetTree().GetRoot() },
        Tree<VizBlock>::PostOrderIterator{},
        [&](const auto& node) { return node->file.name == "fake_file"; });

    QVERIFY(nodeWasAdded == true);
}

REGISTER_TEST(ModelTests)
