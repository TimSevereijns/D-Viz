#include "modelTests.h"

#include <cstdlib>
#include <fstream>
#include <memory>
#include <ostream>
#include <thread>

#include <gsl/gsl_assert>

#include <Scanner/scanningParameters.h>
#include <Scanner/scanningProgress.hpp>
#include <Utilities/operatingSystemSpecific.hpp>
#include <constants.h>

#include "testUtilities.hpp"

namespace
{
    std::filesystem::path PathFromRootToNode(const Tree<VizBlock>::Node& node)
    {
        std::vector<std::reference_wrapper<const std::wstring>> reversePath;
        reversePath.reserve(Tree<VizBlock>::Depth(node));
        reversePath.emplace_back(node->file.name);

        const auto* currentNode = &node;
        while (currentNode->GetParent() && currentNode->GetParent()->GetParent()) {
            currentNode = currentNode->GetParent();
            reversePath.emplace_back(currentNode->GetData().file.name);
        }

        const auto pathFromRoot = std::accumulate(
            std::rbegin(reversePath), std::rend(reversePath), std::wstring{},
            [](const std::wstring& path, const std::wstring& file) {
                constexpr auto slash = L'/';

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
        const typename Tree<VizBlock>::Node& rootNode, const std::wstring_view& fileExtension,
        FileEventType eventType)
    {
        std::vector<FileEvent> allEvents;

        std::for_each(
            Tree<VizBlock>::LeafIterator{ &rootNode }, Tree<VizBlock>::LeafIterator{},
            [&](const auto& node) {
                if (node->file.extension == fileExtension) {
                    const auto path = PathFromRootToNode(node);

                    allEvents.emplace_back(
                        FileEvent{ path.wstring() + node->file.extension, eventType });
                }
            });

        return allEvents;
    }
} // namespace

void ModelTests::initTestCase()
{
    TestUtilities::UnzipTestData(
        "../../Tests/Data/boost-asio.zip", std::filesystem::absolute("../../Tests/Sandbox"));

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
        };

        auto nextNotification = m_sampleNotifications.back();
        m_sampleNotifications.pop_back();

        return std::move(nextNotification);
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

    auto visualizationParameters = Settings::VisualizationParameters{};
    visualizationParameters.rootDirectory = L"";
    visualizationParameters.minimumFileSize = 0u;
    visualizationParameters.onlyShowDirectories = false;

    const Tree<VizBlock>::Node* rootNode = m_tree->GetRoot();
    m_model->HighlightDescendants(*rootNode, visualizationParameters);

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
            return (node->file.name + node->file.extension) == L"endpoint.ipp";
        });

    m_model->HighlightAncestors(*target);

    QCOMPARE(
        static_cast<std::int32_t>(m_model->GetHighlightedNodes().size()),
        static_cast<std::int32_t>(4));
}

void ModelTests::HighlightAllMatchingExtensions()
{
    QVERIFY(m_model->GetHighlightedNodes().size() == 0);

    auto visualizationParameters = Settings::VisualizationParameters{};
    visualizationParameters.rootDirectory = L"";
    visualizationParameters.minimumFileSize = 0u;
    visualizationParameters.onlyShowDirectories = false;

    constexpr auto shouldSearchFiles{ true };
    constexpr auto shouldSearchDirectories{ false };

    m_model->HighlightMatchingFileName(
        L".hpp", visualizationParameters, shouldSearchFiles, shouldSearchDirectories);

    const auto headerCount = std::count_if(
        Tree<VizBlock>::PostOrderIterator{ m_tree->GetRoot() }, Tree<VizBlock>::PostOrderIterator{},
        [](const auto& node) { return node->file.extension == L".hpp"; });

    QCOMPARE(
        static_cast<std::int32_t>(m_model->GetHighlightedNodes().size()),
        static_cast<std::int32_t>(headerCount));
}

void ModelTests::ToggleFileMonitoring()
{
    m_sampleNotifications = std::vector<FileEvent>{ { "spawn.hpp", FileEventType::TOUCHED } };

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
    TestSingleNotification(FileEventType::TOUCHED);
}

void ModelTests::TrackSingleFileDeletion()
{
    TestSingleNotification(FileEventType::DELETED);
}

void ModelTests::TrackSingleFileRename()
{
    TestSingleNotification(FileEventType::RENAMED);
}

void ModelTests::TrackMultipleDeletions()
{
    QVERIFY(m_tree != nullptr);

    m_sampleNotifications = SelectAllFiles(*m_tree->GetRoot(), L".ipp", FileEventType::DELETED);

    m_model->StartMonitoringFileSystem();

    const auto totalNotifications = m_sampleNotifications.size();
    auto processedNotifications{ 0u };

    const auto startTime = std::chrono::high_resolution_clock::now();

    while (processedNotifications != totalNotifications) {
        std::optional<FileEvent> notification = m_model->FetchNextModelChange();

        if (notification) {
            ++processedNotifications;

            QCOMPARE(notification->eventType, FileEventType::DELETED);
            QCOMPARE(notification->path.extension(), L".ipp");
        }

        const auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - startTime);

        if (elapsedTime > std::chrono::milliseconds(500)) {
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
        std::vector<FileEvent>{ { targetFile.string(), FileEventType::DELETED } };

    const auto foundTargetNode = std::any_of(
        Tree<VizBlock>::PostOrderIterator{ m_model->GetTree().GetRoot() },
        Tree<VizBlock>::PostOrderIterator{},
        [&](const auto& node) { return node->file.name == L"basic_socket"; });

    QVERIFY(foundTargetNode == true);

    m_model->StartMonitoringFileSystem();
    m_model->WaitForNextModelChange();
    m_model->RefreshTreemap();
    m_model->StopMonitoringFileSystem();

    const auto wasTargetNodeRemoved = std::none_of(
        Tree<VizBlock>::PostOrderIterator{ m_model->GetTree().GetRoot() },
        Tree<VizBlock>::PostOrderIterator{},
        [&](const auto& node) { return node->file.name == L"basic_socket"; });

    QVERIFY(wasTargetNodeRemoved == true);
}

void ModelTests::ApplyFileCreation()
{
    std::filesystem::path absolutePathToRoot = m_model->GetTree().GetRoot()->GetData().file.name;
    std::filesystem::path targetFile = absolutePathToRoot / "fake_file.hpp";

    m_sampleNotifications = std::vector<FileEvent>{ { targetFile, FileEventType::CREATED } };

    const auto nodeDoesNotExist = std::none_of(
        Tree<VizBlock>::PostOrderIterator{ m_model->GetTree().GetRoot() },
        Tree<VizBlock>::PostOrderIterator{},
        [&](const auto& node) { return node->file.name == L"fake_file"; });

    QVERIFY(nodeDoesNotExist == true);

    m_model->StartMonitoringFileSystem();
    m_model->WaitForNextModelChange();
    m_model->RefreshTreemap();
    m_model->StopMonitoringFileSystem();

    const auto nodeWasAdded = std::any_of(
        Tree<VizBlock>::PostOrderIterator{ m_model->GetTree().GetRoot() },
        Tree<VizBlock>::PostOrderIterator{},
        [&](const auto& node) { return node->file.name == L"fake_file"; });

    QVERIFY(nodeWasAdded == true);
}

REGISTER_TEST(ModelTests) // NOLINT
