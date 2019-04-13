#include "modelTester.h"

#include <fstream>
#include <ostream>

#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/optional.hpp>
#include <gsl/gsl_assert>

#include <Scanner/scanningParameters.h>
#include <Scanner/scanningProgress.hpp>
#include <bootstrapper.hpp>
#include <constants.h>

namespace
{
    void SetupTestData(
        const std::experimental::filesystem::path& zipFile,
        const std::experimental::filesystem::path& directory)
    {
        if (std::experimental::filesystem::exists(directory) &&
            std::experimental::filesystem::is_directory(directory)) {
            std::experimental::filesystem::remove_all(directory);
        }

        std::ifstream inputStream{ zipFile, std::ios_base::in | std::ios_base::binary };

        boost::iostreams::filtering_streambuf<boost::iostreams::input> zipStream;
        zipStream.push(boost::iostreams::gzip_decompressor{});
        zipStream.push(inputStream);

        std::ofstream outputStream{ directory, std::ios_base::out | std::ios_base::binary };

        try {
            boost::iostreams::copy(zipStream, outputStream);
        } catch (const std::exception& exception) {
            std::cout << exception.what() << std::endl;
        }
    }

    void VerifyExistenceOfSampleDirectory(const std::experimental::filesystem::path& directory)
    {
        if (std::experimental::filesystem::exists(directory) &&
            std::experimental::filesystem::is_directory(directory)) {
            return;
        }

        std::cout << "Please unzip boost-asio.zip manually, and re-run tests." << std::endl;
    }
} // namespace

void ModelTester::initTestCase()
{
    Bootstrapper::RegisterMetaTypes();
    Bootstrapper::InitializeLogs();

    // VerifyExistenceOfSampleDirectory(m_sampleDirectory);
    SetupTestData("../../Tests/boost-asio.zip", m_sampleDirectory);

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

    m_model = std::make_unique<SquarifiedTreeMap>(
        std::make_unique<MockFileMonitor>([&] { return m_sampleNotification; }), m_sampleDirectory);

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
    m_sampleNotification = FileChangeNotification{ "spawn.hpp", FileModification::TOUCHED };

    QCOMPARE(m_model->IsFileSystemBeingMonitored(), false);
    m_model->StartMonitoringFileSystem();
    QCOMPARE(m_model->IsFileSystemBeingMonitored(), true);
    m_model->StopMonitoringFileSystem();
    QCOMPARE(m_model->IsFileSystemBeingMonitored(), false);
}

void ModelTester::TrackFileModification()
{
    QVERIFY(m_tree != nullptr);

    // The following notification will be read by the MockFileMonitor and sent to the model. Note
    // that the provided path has to be a relative path to an actual file in the sample directory.
    // If the path doesn't exist then we can't locate a matching node in the tree.
    m_sampleNotification = FileChangeNotification{ "spawn.hpp", FileModification::TOUCHED };

    m_model->StartMonitoringFileSystem();

    boost::optional<FileChangeNotification> notification;
    while (!notification) {
        notification = m_model->FetchNextFileSystemChange();
    }

    const auto modifiedNode = notification->node;
    const std::experimental::filesystem::path path{ "spawn.hpp" };

    QCOMPARE(notification->relativePath, path);
    QCOMPARE(notification->status, FileModification::TOUCHED);
    QCOMPARE(modifiedNode->GetData().file.name, std::wstring{ L"spawn" });
    QCOMPARE(modifiedNode->GetData().file.extension, std::wstring{ L".hpp" });

    m_model->StopMonitoringFileSystem();
}

REGISTER_TEST(ModelTester) // NOLINT
