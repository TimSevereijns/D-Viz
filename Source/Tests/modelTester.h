#include <QString>
#include <QtTest>

#include "mockfilemonitor.h"
#include "multiTestHarness.h"

#include <DriveScanner/driveScanner.h>
#include <Visualizations/fileChangeNotification.hpp>
#include <Visualizations/squarifiedTreemap.h>

#include <iostream>
#include <memory>

class ModelTester : public QObject
{
   Q_OBJECT

private slots:

   /**
    * @brief This preamble is run only per Model Tester class instance.
    */
   void initTestCase();

   /**
    * @brief This preamble is run before each test.
    */
   void init();

   /**
    * @brief During the drive scanning progress, the progress callback should be invoked.
    */
   void ProgressCallbackIsInvoked();

   /**
    * @brief After the scan completes and the data is parsed, the model should be populated.
    */
   void ModelIsPopulated();

   /**
    * @brief ScanningProgressDataIsCorrect
    */
   void ScanningProgressDataIsCorrect();

   /**
    * @brief SelectingNodes
    */
   void SelectingNodes();

   /**
    * @brief HighlightDescendants
    */
   void HighlightDescendants();

   /**
    * @brief HighlightAncestors
    */
   void HighlightAncestors();

   /**
    * @brief HighlightAllMatchingExtensions
    */
   void HighlightAllMatchingExtensions();

   /**
    * @brief ToggleFileMonitoring
    */
   void ToggleFileMonitoring();

   /**
    * @brief TrackFileModification
    */
   void TrackFileModification();

private:

   FileChangeNotification m_sampleNotification;

   const std::experimental::filesystem::path m_sampleDirectory{ "../../Tests/asio" };
   std::experimental::filesystem::path m_path{ m_sampleDirectory };

   DriveScanner m_scanner;

   std::uintmax_t m_bytesScanned;
   std::uintmax_t m_filesScanned;
   std::uintmax_t m_directoriesScanned;

   std::uint32_t m_progressCallbackInvocations{ 0 };

   std::shared_ptr<Tree<VizBlock>> m_tree{ nullptr };
   std::unique_ptr<SquarifiedTreeMap> m_model{ nullptr };
};
