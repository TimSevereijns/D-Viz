#include "modelTester.h"

#include <cstdlib>
#include <fstream>
#include <memory>
#include <ostream>
#include <thread>

#include <gsl/gsl_assert>

#include <Scanner/scanningParameters.h>
#include <Scanner/scanningProgress.hpp>
#include <Utilities/operatingSystemSpecific.hpp>
#include <bootstrapper.hpp>
#include <constants.h>

namespace
{
    /**
     * @brief Decompressing a ZIP archive is such a pain in C++, that I'd rather make a potentially
     * unsafe call to a Python script than try to integrate ZLib. This is just test code, after all.
     *
     * @param[in] zipFile           The path to the ZIP archive.
     * @param[in] outputDirectory   The location we'd at which we'd like to store the decompressed
     *                              data.
     */
    void UnzipTestData(
        const std::filesystem::path& zipFile, const std::filesystem::path& outputDirectory)
    {
        const std::string script = "../../Tests/Scripts/unzipTestData.py";

#if defined(Q_OS_WIN)
        const std::string command = "python " + script + " --input " + zipFile.string() +
                                    " --output " + outputDirectory.string();
#elif defined(Q_OS_LINUX)
        const std::string command = "python3 " + script + " --input " + zipFile.string() +
                                    " --output " + outputDirectory.string();
#endif // Q_OS_LINUX

        std::system(command.c_str());
    }

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

void ModelTester::initTestCase()
{
    Bootstrapper::RegisterMetaTypes();
    Bootstrapper::InitializeLogs();

    UnzipTestData("../../Tests/Data/boost-asio.zip", "../../Tests/Sandbox");

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

void ModelTester::init()
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

void ModelTester::ProgressCallbackIsInvoked()
{
    QVERIFY(m_progressCallbackInvocations > 0); //< Scanning time determines exact count.
}

void ModelTester::ModelIsPopulated()
{
    const auto& tree = m_model->GetTree();

    // Number of items in sample directory:
    QCOMPARE(static_cast<unsigned long>(tree.Size()), 490ul);
}

void ModelTester::ScanningProgressDataIsCorrect()
{
    // Counts as seen in Windows File Explorer:
    QCOMPARE(static_cast<unsigned long>(m_bytesScanned), 3'407'665ul);
    QCOMPARE(static_cast<unsigned long>(m_filesScanned), 469ul);
    QCOMPARE(static_cast<unsigned long>(m_directoriesScanned), 20ul);
}

void ModelTester::SelectingNodes()
{
    QVERIFY(m_model->GetSelectedNode() == nullptr);

    const Tree<VizBlock>::Node* sampleNode = m_tree->GetRoot();
    m_model->SelectNode(*sampleNode);
    QVERIFY(m_model->GetSelectedNode() == sampleNode);

    m_model->ClearSelectedNode();
    QVERIFY(m_model->GetSelectedNode() == nullptr);
}

void ModelTester::HighlightDescendants()
{
    QVERIFY(m_model->GetHighlightedNodes().size() == 0);

    auto visualizationParameters = Settings::VisualizationParameters{};
    visualizationParameters.rootDirectory = L"";
    visualizationParameters.minimumFileSize = 0u;
    visualizationParameters.onlyShowDirectories = false;
    visualizationParameters.useDirectoryGradient = false;

    const Tree<VizBlock>::Node* rootNode = m_tree->GetRoot();
    m_model->HighlightDescendants(*rootNode, visualizationParameters);

    const auto leafCount = std::count_if(
        Tree<VizBlock>::LeafIterator{ rootNode }, Tree<VizBlock>::LeafIterator{},
        [](const auto&) { return true; });

    QCOMPARE(
        static_cast<std::int32_t>(m_model->GetHighlightedNodes().size()),
        static_cast<std::int32_t>(leafCount));
}

void ModelTester::HighlightAncestors()
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

void ModelTester::HighlightAllMatchingExtensions()
{
    QVERIFY(m_model->GetHighlightedNodes().size() == 0);

    auto visualizationParameters = Settings::VisualizationParameters{};
    visualizationParameters.rootDirectory = L"";
    visualizationParameters.minimumFileSize = 0u;
    visualizationParameters.onlyShowDirectories = false;
    visualizationParameters.useDirectoryGradient = false;

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

void ModelTester::ToggleFileMonitoring()
{
    m_sampleNotifications = std::vector<FileEvent>{ { "spawn.hpp", FileEventType::TOUCHED } };

    QCOMPARE(m_model->IsFileSystemBeingMonitored(), false);
    m_model->StartMonitoringFileSystem();
    QCOMPARE(m_model->IsFileSystemBeingMonitored(), true);
    m_model->StopMonitoringFileSystem();
    QCOMPARE(m_model->IsFileSystemBeingMonitored(), false);
}

void ModelTester::TestSingleNotification(FileEventType eventType)
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

void ModelTester::TrackSingleFileModification()
{
    TestSingleNotification(FileEventType::TOUCHED);
}

void ModelTester::TrackSingleFileDeletion()
{
    TestSingleNotification(FileEventType::DELETED);
}

void ModelTester::TrackSingleFileRename()
{
    TestSingleNotification(FileEventType::RENAMED);
}

void ModelTester::TrackMultipleDeletions()
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

void ModelTester::ApplyFileDeletion()
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

void ModelTester::ApplyFileCreation()
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

REGISTER_TEST(ModelTester) // NOLINT
