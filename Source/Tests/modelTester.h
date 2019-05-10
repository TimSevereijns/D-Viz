#include <QString>
#include <QtTest>

#include "mockFileMonitor.h"
#include "multiTestHarness.h"

#include <Scanner/Monitor/fileChangeNotification.hpp>
#include <Scanner/driveScanner.h>
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
     * @brief Verifies that the progress callback is correctly invoked.
     */
    void ProgressCallbackIsInvoked();

    /**
     * @brief Verifies that the model is correctly populated after the scan completes and after the
     * data is parsed.
     */
    void ModelIsPopulated();

    /**
     * @brief Verifies that scanning progress is properly reported.
     */
    void ScanningProgressDataIsCorrect();

    /**
     * @brief Verifies that the correct node is selected.
     */
    void SelectingNodes();

    /**
     * @brief Verifies that node descendants are correctly highlighted.
     */
    void HighlightDescendants();

    /**
     * @brief Verifies that node ancestors are correctly highlighted.
     */
    void HighlightAncestors();

    /**
     * @brief Verifies that all nodes with a given extension are correctly highlighted.
     */
    void HighlightAllMatchingExtensions();

    /**
     * @brief Verifies that file monitoring is correctly enabled and disabled.
     */
    void ToggleFileMonitoring();

    /**
     * @brief Verifies that file system changes are correctly detected and tracked.
     */
    void TrackSingleFileModification();

    /**
     * @brief Verifies that file deletions are correctly detected and tracked.
     */
    void TrackSingleFileDeletion();

    /**
     * @brief Verifies that file deletions are correctly detected and tracked.
     */
    void TrackSingleFileRename();

    /**
     * @brief TrackMultipleDeletions
     */
    void TrackMultipleDeletions();

    /**
     * @brief ApplyFileDeletion
     */
    void ApplyFileDeletion();

    /**
     * @brief ApplyFileCreation
     */
    void ApplyFileCreation();

  private:
    void TestSingleNotification(FileEventType eventType);

    std::vector<FileEvent> m_sampleNotifications;

    std::filesystem::path m_sampleDirectory{ std::filesystem::absolute(
        "../../Tests/Sandbox/asio") };

    DriveScanner m_scanner;

    std::uintmax_t m_bytesScanned{ 0 };
    std::uintmax_t m_filesScanned{ 0 };
    std::uintmax_t m_directoriesScanned{ 0 };

    std::uint32_t m_progressCallbackInvocations{ 0 };

    std::shared_ptr<Tree<VizBlock>> m_tree{ nullptr };
    std::unique_ptr<SquarifiedTreeMap> m_model{ nullptr };
};
