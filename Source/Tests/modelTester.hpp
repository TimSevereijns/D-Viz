#include <QString>
#include <QtTest>

#include <iostream>

#include <bootstrapper.hpp>
#include <constants.h>
#include <DataStructs/driveScanningParameters.h>
#include <DataStructs/scanningProgress.hpp>
#include <DriveScanner/driveScanner.h>
#include <Visualizations/squarifiedTreemap.h>

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

   void ProgressCallbackIsInvoked();
   void SelectedNodeIsNull();
   void ModelIsPopulated();

private:

   std::experimental::filesystem::path m_path{ pathToTestData };

   DriveScanner m_scanner;
   ScanningProgress m_scanningProgress;

   std::uint32_t m_progressCallbackInvocations{ 0 };

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
      (const ScanningProgress& /*progress*/, std::shared_ptr<Tree<VizBlock>> tree)
   {
      m_model = std::make_unique<SquarifiedTreeMap>(m_path);
      m_model->Parse(tree);
   };

   QSignalSpy completionSpy(&m_scanner, &DriveScanner::Finished);

   const DriveScanningParameters parameters{ m_path, progressCallback, completionCallback };
   m_scanner.StartScanning(parameters);

   completionSpy.wait(3'000);
}

void ModelTester::ProgressCallbackIsInvoked()
{
   QVERIFY(m_progressCallbackInvocations > 0);
}

void ModelTester::SelectedNodeIsNull()
{
   QVERIFY(m_model->GetSelectedNode() == nullptr);
}

void ModelTester::ModelIsPopulated()
{
   const auto tree = m_model->GetTree();
   QCOMPARE(static_cast<int>(tree.Size()), 490); //< Number of items in "asio" sample directory.
}

QTEST_MAIN(ModelTester)

//#include "modelTester.moc"
