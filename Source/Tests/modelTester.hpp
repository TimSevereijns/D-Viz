#include <QString>
#include <QtTest>

#include <bootstrapper.hpp>
#include <constants.h>
#include <DataStructs/driveScanningParameters.h>
#include <DataStructs/scanningProgress.hpp>
#include <DriveScanner/driveScanner.h>
#include <Visualizations/squarifiedTreemap.h>

class ModelTester : public QObject
{
   Q_OBJECT

private slots:

   void initTestCase();

   void ProgressCallbackIsInvocaked();
   void SelectedNodeIsNull();

private:

   std::experimental::filesystem::path m_path{ L"C:\\Qt" };

   DriveScanner m_scanner;
   ScanningProgress m_scanningProgress;

   std::uint32_t m_progressCallbackInvocations{ 0 };

   std::unique_ptr<SquarifiedTreeMap> m_model{ nullptr };
};

void ModelTester::initTestCase()
{
   Bootstrapper::RegisterMetaTypes();
   Bootstrapper::InitializeLog();

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

   DriveScanningParameters parameters{ m_path, progressCallback, completionCallback };
   m_scanner.StartScanning(parameters);

   QSignalSpy completionSpy(&m_scanner, &DriveScanner::Finished);
   completionSpy.wait(30000);
}

void ModelTester::ProgressCallbackIsInvocaked()
{
   QVERIFY(m_progressCallbackInvocations > 0);
}

void ModelTester::SelectedNodeIsNull()
{
   QVERIFY(m_model->GetSelectedNode() == nullptr);
}

QTEST_MAIN(ModelTester)

//#include "modelTester.moc"
