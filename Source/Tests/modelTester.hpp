#include <QString>
#include <QtTest>

#include <spdlog/spdlog.h>

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>

#include <constants.h>
#include <DataStructs/driveScanningParameters.h>
#include <DataStructs/scanningProgress.hpp>
#include <DriveScanner/driveScanner.h>
#include <Visualizations/squarifiedTreemap.h>

namespace
{
   /**
    * @brief Returns a wide string if on Windows, and returns a narrow string on Unix.
   */
   auto ToFilenameString(const std::experimental::filesystem::path& path)
   {
      if constexpr (std::is_same_v<spdlog::filename_t, std::wstring>)
      {
         return path.wstring();
      }

      if constexpr (std::is_same_v<spdlog::filename_t, std::string>)
      {
         return path.string();
      }
   }

   /**
    * @brief Performs all the steps necessary to initialize and start the log.
    */
   void InitializeLog()
   {
      const auto defaultLogPath =
         std::experimental::filesystem::current_path().append("test-log.txt");

      const auto& defaultLog = spdlog::basic_logger_mt(
         Constants::Logging::DEFAULT_LOG,
         ToFilenameString(defaultLogPath));

      const auto fileLogPath =
         std::experimental::filesystem::current_path().append("test-fileSytem.txt");

      const auto& filesystemLog = spdlog::basic_logger_mt(
         Constants::Logging::FILESYSTEM_LOG,
         ToFilenameString(fileLogPath));

      defaultLog->info("--------------------------------");
      defaultLog->info("Starting D-Viz...");

      filesystemLog->info("--------------------------------");
      filesystemLog->info("Starting D-Viz...");
   }

   /**
    * @brief Registers the types that we'd like pass through the Qt signaling framework.
    */
   void RegisterMetaTypes()
   {
      qRegisterMetaType<std::uintmax_t>("std::uintmax_t");
      qRegisterMetaType<std::shared_ptr<Tree<VizBlock>>>("std::shared_ptr<Tree<VizBlock>>");
   }
}

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
   RegisterMetaTypes();
   InitializeLog();

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
