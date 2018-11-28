#include <QString>
#include <QtTest>

#include "multiTestHarness.h"

#include <iostream>

#include <bootstrapper.hpp>
#include <constants.h>
#include <DataStructs/driveScanningParameters.h>
#include <DataStructs/scanningProgress.hpp>
#include <DriveScanner/driveScanner.h>
#include "Visualizations/fileChangeNotification.hpp"
#include <Visualizations/squarifiedTreemap.h>

#include <boost/optional.hpp>
#include <gsl/gsl_assert>

#if defined(Q_OS_WIN)
   #include "Visualizations/windowsFileMonitor.h"
#elif defined(Q_OS_LINUX)
   #include "Visualizations/linuxFileMonitor.h"
#endif // Q_OS_LINUX

namespace Detail
{
#if defined(Q_OS_WIN)
   using FileSystemMonitor = WindowsFileMonitor;
#elif defined(Q_OS_LINUX)
   using FileSystemMonitor = LinuxFileMonitor;
#endif // Q_OS_LINUX

   const auto sampleDirectory = std::experimental::filesystem::path{ "../../Tests/asio" };

   void VerifyExistenceOfSampleDirectory()
   {
      if (std::experimental::filesystem::exists(sampleDirectory)
         && std::experimental::filesystem::is_directory(sampleDirectory))
      {
         return;
      }

      std::cout << "Please unzip boost-asio.zip manually, and re-run tests." << std::endl;
   }

   /**
    * @brief The MockFileMonitor class
    */
   class MockFileMonitor : public FileMonitorBase
   {
   public:

      MockFileMonitor(std::function<FileChangeNotification ()> notificationGenerator)
         : m_notificationGenerator{ std::move(notificationGenerator) }
      {
      }

      ~MockFileMonitor() noexcept override
      {
         if (m_workerThread.joinable())
         {
            m_workerThread.join();
         }
      }

      void Start(
         const std::experimental::filesystem::path& path,
         const std::function<void (FileChangeNotification&&)>& callback) override
      {
         m_pathToMonitor = path;
         m_callback = callback;
         m_workerThread = std::thread{ [&]{ SendFakeNotification(); } };
         m_isActive = true;
      }

      void Stop() override
      {
         m_isActive = false;
      }

      bool IsActive() const override
      {
         return m_isActive;
      }

      std::experimental::filesystem::path GetPathToMonitor() const noexcept
      {
         return m_pathToMonitor;
      }

   private:

      void SendFakeNotification() const noexcept
      {
         auto notification = m_notificationGenerator();
         m_callback(std::move(notification));
      }

      std::function<FileChangeNotification ()> m_notificationGenerator;

      std::function<void (FileChangeNotification&&)> m_callback;

      std::thread m_workerThread;

      std::experimental::filesystem::path m_pathToMonitor;

      bool m_isActive{ false };
   };
}

class ModelTester : public QObject
{
   Q_OBJECT

private slots:

   void initTestCase();
   void init();

   void ProgressCallbackIsInvoked();
   void ModelIsPopulated();
   void ScanningProgressDataIsCorrect();
   void SelectingNodes();
   void HighlightDescendants();
   void HighlightAncestors();
   void HighlightAllMatchingExtensions();
   void ToggleFileMonitoring();
   void TrackFileModification();

private:

   FileChangeNotification m_sampleNotification;

   std::experimental::filesystem::path m_path{ Detail::sampleDirectory };

   DriveScanner m_scanner;

   std::uintmax_t m_bytesScanned;
   std::uintmax_t m_filesScanned;
   std::uintmax_t m_directoriesScanned;

   std::uint32_t m_progressCallbackInvocations{ 0 };

   std::shared_ptr<Tree<VizBlock>> m_tree{ nullptr };
   std::unique_ptr<SquarifiedTreeMap> m_model{ nullptr };
};

/**
 * @brief This preamble is run only once.
 */
void ModelTester::initTestCase()
{
   Bootstrapper::RegisterMetaTypes();
   Bootstrapper::InitializeLogs();

   Detail::VerifyExistenceOfSampleDirectory();

   const auto progressCallback = [&] (const ScanningProgress& /*progress*/)
   {
      m_progressCallbackInvocations++;
   };

   const auto completionCallback = [&]
      (const ScanningProgress& progress, std::shared_ptr<Tree<VizBlock>> tree)
   {
      QVERIFY(tree != nullptr);

      m_bytesScanned = progress.bytesProcessed.load();
      m_filesScanned = progress.filesScanned.load();
      m_directoriesScanned = progress.directoriesScanned.load();

      m_tree = std::move(tree);
   };

   QSignalSpy completionSpy(&m_scanner, &DriveScanner::Finished);

   const DriveScanningParameters parameters{ m_path, progressCallback, completionCallback };
   m_scanner.StartScanning(parameters);

   completionSpy.wait(10'000);
}

/**
 * @brief This preamble is run before each test.
 */
void ModelTester::init()
{
   QVERIFY(m_tree != nullptr);

   auto notificationGenerator = [&] { return m_sampleNotification; };

   m_model = std::make_unique<SquarifiedTreeMap>(
      std::make_unique<Detail::MockFileMonitor>(std::move(notificationGenerator)),
      m_path);

   m_model->Parse(m_tree);
}

void ModelTester::ProgressCallbackIsInvoked()
{
   QVERIFY(m_progressCallbackInvocations > 0);  //< Scanning time determines exact count.
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

   auto visualizationParameters = Settings::VisualizationParameters{ };
   visualizationParameters.rootDirectory = L"";
   visualizationParameters.minimumFileSize = 0u;
   visualizationParameters.onlyShowDirectories = false;
   visualizationParameters.useDirectoryGradient = false;

   const Tree<VizBlock>::Node* rootNode = m_tree->GetRoot();
   m_model->HighlightDescendants(*rootNode, visualizationParameters);

   const auto leafCount = std::count_if(
      Tree<VizBlock>::LeafIterator{ rootNode },
      Tree<VizBlock>::LeafIterator{ },
      [] (const auto&) { return true; });

   QCOMPARE(
      static_cast<std::int32_t>(m_model->GetHighlightedNodes().size()),
      static_cast<std::int32_t>(leafCount));
}

void ModelTester::HighlightAncestors()
{
   QVERIFY(m_model->GetHighlightedNodes().size() == 0);

   const auto target = std::find_if(
      Tree<VizBlock>::LeafIterator{ m_tree->GetRoot() },
      Tree<VizBlock>::LeafIterator{ },
      [] (const auto& node)
   {
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

   auto visualizationParameters = Settings::VisualizationParameters{ };
   visualizationParameters.rootDirectory = L"";
   visualizationParameters.minimumFileSize = 0u;
   visualizationParameters.onlyShowDirectories = false;
   visualizationParameters.useDirectoryGradient = false;

   constexpr auto shouldSearchFiles{ true };
   constexpr auto shouldSearchDirectories{ false };

   m_model->HighlightMatchingFileName(
      L".hpp",
      visualizationParameters,
      shouldSearchFiles,
      shouldSearchDirectories);

   const auto headerCount = std::count_if(
      Tree<VizBlock>::PostOrderIterator{ m_tree->GetRoot() },
      Tree<VizBlock>::PostOrderIterator{ },
      [] (const auto& node) { return node->file.extension == L".hpp"; });

   QCOMPARE(
      static_cast<std::int32_t>(m_model->GetHighlightedNodes().size()),
      static_cast<std::int32_t>(headerCount));
}

void ModelTester::ToggleFileMonitoring()
{
   m_sampleNotification = FileChangeNotification
   {
      "spawn.hpp",
      FileModification::TOUCHED,
      std::chrono::high_resolution_clock::now()
   };

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
   m_sampleNotification = FileChangeNotification
   {
      "spawn.hpp",
      FileModification::TOUCHED,
      std::chrono::high_resolution_clock::now()
   };

   m_model->StartMonitoringFileSystem();

   boost::optional<FileChangeNotification> notification;
   while (!notification)
   {
      notification = m_model->FetchNodeUpdate();
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

//#include "modelTester.moc"
