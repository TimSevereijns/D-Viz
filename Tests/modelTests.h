#ifndef MODELTESTS_H
#define MODELTESTS_H

#include <QString>
#include <QtTest>

#include "Mocks/mockFileMonitor.h"
#include "multiTestHarness.h"

#include <Scanner/Monitor/fileChangeNotification.hpp>
#include <Scanner/driveScanner.h>
#include <Visualizations/squarifiedTreemap.h>

#include <memory>

class ModelTests : public QObject
{
    Q_OBJECT
  public:
    ModelTests();

  private slots:

    /**
     * @brief This preamble is run only once for the entire class. All setup work should be done
     * here.
     */
    void initTestCase();

    /**
     * @brief Clean up code for the entire class; called once.
     */
    void cleanupTestCase();

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
     * @brief Verifies that the root path is correctly returned.
     */
    void GetRootPath();

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
    void HighlightAllMatchingFileNames();

    /**
     * @brief Verifies that matching extensions are correctly highlighted.
     */
    void HighlightMatchingFileExtensions();

    /**
     * @brief Verifies that highlights get correctly cleared.
     */
    void ClearHighlightedNodes();

    /**
     * @brief Verifies that bounding boxes are correctly computed.
     */
    void ComputeBoundingBoxes();

    /**
     * @brief Verifies that the path to the selected node is correctly copied to the clipboard.
     */
    void CopyPathToClipboard();

    /**
     * @brief Verifies that the correct node is found when the camera is in front of the targeted
     * node.
     */
    void FindNearestNodeFromFront();

    /**
     * @brief Verifies that the correct node is found when the camera is behind of the targeted
     * node.
     */
    void FindNearestNodeFromBack();

    /**
     * @brief Verifies that the correct node is found when we specify a minimum file size.
     */
    void FindNearestNodeWithSizeLimitations();

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
     * @brief Verifies that a file deletion is correctly applied to the mode once refreshed.
     */
    void ApplyFileDeletion();

    /**
     * @brief Verifies that a file creation is correctly applied to the mode once refreshed.
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

#endif // MODELTESTS_H
