#include "modelTester.h"

#include <cstdlib>
#include <fstream>
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
        const std::experimental::filesystem::path& zipFile,
        const std::experimental::filesystem::path& outputDirectory)
    {
        const std::string script = "../../Tests/Scripts/unzipTestData.py";
        const std::string command = "python3 " + script + " --input " + zipFile.string() +
                                    " --output " + outputDirectory.string();

        std::system(command.c_str());
    }

    std::wstring PathFromRootToNode(const Tree<VizBlock>::Node& node)
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
                if (!path.empty() && path.back() != OS::PREFERRED_SLASH) {
                    return path + OS::PREFERRED_SLASH + file;
                }

                return path + file;
            });

        return pathFromRoot;
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

    const auto notificationGenerator = [&]() -> boost::optional<FileChangeNotification> {
        if (m_sampleNotifications.empty()) {
            return boost::none;
        };

        const auto nextNotification = m_sampleNotifications.back();
        m_sampleNotifications.pop_back();

        return nextNotification;
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
    const auto tree = m_model->GetTree();

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
    m_sampleNotifications =
        std::vector<FileChangeNotification>{ { "spawn.hpp", FileModification::TOUCHED } };

    QCOMPARE(m_model->IsFileSystemBeingMonitored(), false);
    m_model->StartMonitoringFileSystem();
    QCOMPARE(m_model->IsFileSystemBeingMonitored(), true);
    m_model->StopMonitoringFileSystem();
    QCOMPARE(m_model->IsFileSystemBeingMonitored(), false);
}

void ModelTester::TestSingleNotification(FileModification eventType)
{
    QVERIFY(m_tree != nullptr);

    // The following notification will be read by the MockFileMonitor and sent to the model. Note
    // that the provided path has to be a relative path to an actual file in the sample directory.
    // If the path doesn't exist then we can't locate a matching node in the tree.
    m_sampleNotifications = std::vector<FileChangeNotification>{ { "spawn.hpp", eventType } };

    m_model->StartMonitoringFileSystem();
    m_model->WaitForNextChange();

    const auto possibleNotification = m_model->FetchNextFileSystemChange();
    QVERIFY(possibleNotification.is_initialized());

    m_model->StopMonitoringFileSystem();

    const auto modifiedNode = possibleNotification->node;
    const std::experimental::filesystem::path path{ "spawn.hpp" };

    QCOMPARE(possibleNotification->relativePath, path);
    QCOMPARE(possibleNotification->status, eventType);
    QCOMPARE(modifiedNode->GetData().file.name, std::wstring{ L"spawn" });
    QCOMPARE(modifiedNode->GetData().file.extension, std::wstring{ L".hpp" });
}

void ModelTester::TrackSingleFileModification()
{
    TestSingleNotification(FileModification::TOUCHED);
}

void ModelTester::TrackSingleFileDeletion()
{
    TestSingleNotification(FileModification::DELETED);
}

void ModelTester::TrackSingleFileRename()
{
    TestSingleNotification(FileModification::RENAMED);
}

void ModelTester::TrackMultipleDeletions()
{
    QVERIFY(m_tree != nullptr);

    // Simulate the deletion of all ".ipp" files:
    m_sampleNotifications = std::vector<FileChangeNotification>{};

    std::for_each(
        Tree<VizBlock>::LeafIterator{ m_tree->GetRoot() }, Tree<VizBlock>::LeafIterator{},
        [&](const auto& node) {
            if (node->file.extension == L".ipp") {
                const auto path = PathFromRootToNode(node);

                m_sampleNotifications.emplace_back(FileChangeNotification{
                    path + node->file.extension, FileModification::DELETED });
            }
        });

    m_model->StartMonitoringFileSystem();

    boost::optional<FileChangeNotification> notification;

    const auto totalNotifications = m_sampleNotifications.size();
    auto processedNotifications{ 0u };

    const auto startTime = std::chrono::high_resolution_clock::now();
    auto elapsedTime = std::chrono::milliseconds{ 0 };

    while (processedNotifications != totalNotifications &&
           elapsedTime < std::chrono::milliseconds(500)) {
        notification = m_model->FetchNextFileSystemChange();

        if (notification) {
            ++processedNotifications;

            const auto modifiedNode = notification->node;

            QCOMPARE(notification->status, FileModification::DELETED);
            QCOMPARE(modifiedNode->GetData().file.extension, std::wstring{ L".ipp" });
        }

        elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - startTime);
    }

    m_model->StopMonitoringFileSystem();

    QCOMPARE(processedNotifications, totalNotifications);
}

void ModelTester::ApplyFileDeletion()
{
    //    m_sampleNotifications =
    //        std::vector<FileChangeNotification>{ { "spawn.hpp", FileModification::DELETED } };

    //    m_model->StartMonitoringFileSystem();
    //    m_model->WaitForNextChange();

    //    // m_model->RefreshTreemap();
    //    m_model->StopMonitoringFileSystem();

    //    const auto nodeIsGone = std::find_if(
    //        Tree<VizBlock>::LeafIterator{ m_tree->GetRoot() }, Tree<VizBlock>::LeafIterator{},
    //        [&](const auto& node) { return node->file.name == L"spawn"; });

    //    QVERIFY(nodeIsGone);
}

REGISTER_TEST(ModelTester) // NOLINT
