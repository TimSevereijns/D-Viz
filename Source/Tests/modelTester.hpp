#include <QString>
#include <QtTest>

#include "multiTestHarness.h"

#include <iostream>

#include <bootstrapper.hpp>
#include <constants.h>
#include <DataStructs/driveScanningParameters.h>
#include <DataStructs/scanningProgress.hpp>
#include <DriveScanner/driveScanner.h>
#include <Visualizations/squarifiedTreemap.h>

#include <gsl/gsl_assert>

namespace
{
   const auto pathToTestData = std::experimental::filesystem::path{ "../../Tests/asio" };

   void VerifyExistenceOfSampleDirectory()
   {
      if (std::experimental::filesystem::exists(pathToTestData)
         && std::experimental::filesystem::is_directory(pathToTestData))
      {
         return;
      }

      std::cout << "Please unzip boost-asio.zip manually, and re-run tests." << std::endl;
   }
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

private:

   std::experimental::filesystem::path m_path{ pathToTestData };

   DriveScanner m_scanner;

   std::uintmax_t m_bytesScanned;
   std::uintmax_t m_filesScanned;
   std::uintmax_t m_directoriesScanned;

   std::uint32_t m_progressCallbackInvocations{ 0 };

   std::shared_ptr<Tree<VizBlock>> m_tree{ nullptr };
   std::unique_ptr<SquarifiedTreeMap> m_model{ nullptr };
};

void ModelTester::initTestCase()
{
   Bootstrapper::RegisterMetaTypes();
   Bootstrapper::InitializeLogs();

   VerifyExistenceOfSampleDirectory();

   const auto progressCallback = [&] (const ScanningProgress& /*progress*/)
   {
      m_progressCallbackInvocations++;
   };

   const auto completionCallback = [&]
      (const ScanningProgress& progress, std::shared_ptr<Tree<VizBlock>> tree)
   {
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

void ModelTester::init()
{
   Expects(m_tree);

   m_model = std::make_unique<SquarifiedTreeMap>(m_path);
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

   const std::size_t leafCount = std::count_if(
      Tree<VizBlock>::LeafIterator{ rootNode },
      Tree<VizBlock>::LeafIterator{ },
      [] (const auto&) { return true; });

   QCOMPARE(m_model->GetHighlightedNodes().size(), leafCount);
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
      static_cast<std::uint64_t>(m_model->GetHighlightedNodes().size()),
      static_cast<std::uint64_t>(std::size_t{ 4u }));
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

   const std::size_t headerCount = std::count_if(
      Tree<VizBlock>::PostOrderIterator{ m_tree->GetRoot() },
      Tree<VizBlock>::PostOrderIterator{ },
      [] (const auto& node) { return node->file.extension == L".hpp"; });

   QCOMPARE(
      static_cast<std::uint64_t>(m_model->GetHighlightedNodes().size()),
      static_cast<std::uint64_t>(headerCount));
}

REGISTER_TEST(ModelTester)

//#include "modelTester.moc"
