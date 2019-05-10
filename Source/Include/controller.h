#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "constants.h"

#include "Scanner/Monitor/fileChangeNotification.hpp"
#include "Scanner/driveScanner.h"
#include "Scene/light.h"
#include "Settings/settingsManager.h"
#include "Visualizations/vizBlock.h"
#include "Windows/mainWindow.h"

#include <Tree/Tree.hpp>

#include <memory>
#include <vector>

#include <QVector3D>

struct FileEvent;
struct ScanningProgress;

class GLCanvas;

class Controller
{
  public:
    Controller();

    /**
     * @brief Starts the UI.
     */
    void LaunchUI();

    /**
     * @brief Scans the drive.
     *
     * @param[in] parameters      Drive scanning parameters; where to start, et cetera.
     */
    void ScanDrive(Settings::VisualizationParameters& parameters);

    /**
     * @returns True if the visualization is not null;
     */
    bool HasModelBeenLoaded() const;

    /**
     * @returns A pointer to the selected node.
     */
    const Tree<VizBlock>::Node* GetSelectedNode() const;

    /**
     * @returns A reference to the tree that represents the most recent drive scan.
     */
    Tree<VizBlock>& GetTree();

    /**
     * @overload
     */
    const Tree<VizBlock>& GetTree() const;

    /**
     * @returns A reference to the currently highlighted nodes. Highlighted nodes are distinct
     * from the selected node (of which there can be only one).
     */
    const std::vector<const Tree<VizBlock>::Node*>& GetHighlightedNodes() const;

    /**
     * @brief Determines whether the node is currently highlighted.
     *
     * @param[in] node            The node whose extension is to be highlighted.
     *
     * @returns True if the given node is currently highlighted; false otherwise.
     */
    bool IsNodeHighlighted(const Tree<VizBlock>::Node& node) const;

    /**
     * @brief Searches the treemap for the search query contained within the search box.
     *
     * @param[in] searchQuery              String to search against.
     * @param[in] deselectionCallback      UI callback to clear selection highlights.
     * @param[in] selectionCallback        UI callback to highlight matching nodes on the canvas.
     * @param[in] shouldSearchFiles        Pass in true to search files.
     * @param[in] shouldSearchDirectories  Pass in true to search directories.
     */
    void SearchTreeMap(
        const std::wstring& searchQuery,
        const std::function<void(std::vector<const Tree<VizBlock>::Node*>&)>& deselectionCallback,
        const std::function<void(std::vector<const Tree<VizBlock>::Node*>&)>& selectionCallback,
        bool shouldSearchFiles, bool shouldSearchDirectories);

    /**
     * @brief Highlights all nodes in the tree whose extension matches that of the passed in node.
     *
     * @param[in] sampleNode      The node whose extension is to be highlighted.
     * @param[in] callback        UI callback to highlight matching nodes on the canvas.
     */
    void HighlightAllMatchingExtensions(
        const Tree<VizBlock>::Node& sampleNode,
        const std::function<void(std::vector<const Tree<VizBlock>::Node*>&)>& callback);

    /**
     * @brief Highlights all nodes that descendant from the passed in node.
     *
     * @param[in] node            The node whose descendants to highlight.
     * @param[in] callback        UI callback to highlight matching nodes on the canvas.
     */
    void HighlightDescendants(
        const Tree<VizBlock>::Node& node,
        const std::function<void(std::vector<const Tree<VizBlock>::Node*>&)>& callback);

    /**
     * @brief Highlights all nodes that are ancestors of the passed in node.
     *
     * @param[in] node            The node whose ancestors are to be highlighted.
     * @param[in] callback        UI callback to highlight matching nodes on the canvas.
     */
    void HighlightAncestors(
        const Tree<VizBlock>::Node& node,
        const std::function<void(std::vector<const Tree<VizBlock>::Node*>&)>& callback);

    /**
     * @brief Clears the selected node, and restores the color of that selected node back to its
     * unselected color.
     */
    void ClearSelectedNode();

    /**
     * @brief Clears all highlighted nodes, and restores the color of any highlighted nodes back
     * to its unhighlighted color.
     *
     * @param[in] callback        UI callback to highlight matching nodes on the canvas.
     */
    void ClearHighlightedNodes(
        const std::function<void(std::vector<const Tree<VizBlock>::Node*>&)>& callback);

    /**
     * @brief Selects the passed in node.
     *
     * @param[in] node               A pointer to the node to be selected.
     * @param[in] selectorCallback   UI callback to highlight matching node on the canvas.
     */
    void SelectNode(
        const Tree<VizBlock>::Node& node,
        const std::function<void(const Tree<VizBlock>::Node&)>& selectorCallback);

    /**
     * @brief Selects the passed in node, and updates the status bar with information about the
     * node.
     *
     * @param[in] node               A pointer to the node to be selected.
     * @param[in] selectorCallback   UI callback to highlight matching node on the canvas.
     */
    void SelectNodeAndUpdateStatusBar(
        const Tree<VizBlock>::Node& node,
        const std::function<void(const Tree<VizBlock>::Node&)>& selectorCallback);

    /**
     * @brief Uses the passed in ray to select the nearest node from the perspective of the
     * camera.
     *
     * @todo Remove the camera object, and pass in the camera's position instead.
     *
     * @param[in] camera              The camera from which the ray was shot.
     * @param[in] ray                 The picking ray.
     * @param[in] deselectionCallback UI callback to clear the previous selection.
     * @param[in] selectionCallback   UI callback to select the matching nodes on the canvas.
     */
    void SelectNodeViaRay(
        const Camera& camera, const Ray& ray,
        const std::function<void(const Tree<VizBlock>::Node&)>& deselectionCallback,
        const std::function<void(const Tree<VizBlock>::Node&)>& selectionCallback);

    /**
     * @brief Helper function to print visualization metadata to the bottom status bar.
     */
    void PrintMetadataToStatusBar();

    /**
     * @brief Converts the given size of the file from bytes to the most human readable units.
     *
     * @param[in] sizeInBytes     The size (in bytes) to be converted to a more appropriate
     *                            unit.
     * @param[in] prefix          The desired prefix.
     *
     * @returns A std::pair encapsulating the converted file size, and corresponding unit readout
     * string.
     */
    static std::pair<double, std::wstring>
    ConvertFileSizeToNumericPrefix(std::uintmax_t sizeInBytes, Constants::FileSize::Prefix prefix);

    /**
     * @brief Computes the absolute file path of the selected node by traveling up tree.
     *
     * @param[in] node            The selected node.
     *
     * @returns The absolute file path.
     */
    static std::wstring ResolveCompleteFilePath(const Tree<VizBlock>::Node& node);

    /**
     * @brief Prints selection details to the main window's status bar.
     */
    void DisplaySelectionDetails();

    /**
     * @brief Whether to allow the user to interact with the UI.
     *
     * @param allowInteraction    The new state.
     */
    void AllowUserInteractionWithModel(bool allowInteraction);

    /**
     * @returns True if the user is allow to select or highlight nodes in the visualization model.
     */
    bool IsUserAllowedToInteractWithModel() const;

    /**
     * @brief Saves the results of the scan.
     *
     * @param[in] progress        @see ScanningProgress
     */
    void SaveScanMetadata(const ScanningProgress& progress);

    /**
     * @returns The options manager.
     */
    Settings::Manager& GetSettingsManager();

    /**
     * @overload
     */
    const Settings::Manager& GetSettingsManager() const;

    /**
     * @brief GetRootPath
     * @return
     */
    std::filesystem::path GetRootPath() const;

    /**
     * @brief Starts monitoring the file system. This is only possible after the first scan.
     */
    void StartMonitoringFileSystem();

    /**
     * @brief IsFileSystemBeingMonitored
     *
     * @return
     */
    bool IsFileSystemBeingMonitored() const;

    /**
     * @brief Fetches oldest, unprocessed file system change notification.
     */
    boost::optional<FileEvent> FetchFileModification();

  private:
    template <typename NodeSelectorType>
    void ProcessSelection(
        const NodeSelectorType& nodeSelector,
        const std::function<void(std::vector<const Tree<VizBlock>::Node*>&)>& callback);

    void ComputeProgress(const ScanningProgress& progress);

    bool m_allowInteractionWithModel{ false };

    Settings::Manager m_settingsManager;

    std::unique_ptr<MainWindow> m_view{ nullptr };

    std::unique_ptr<VisualizationModel> m_model{ nullptr };

    DriveScanner m_scanner;

    std::uint64_t m_occupiedDiskSpace{ 0u };
};

#endif // CONTROLLER_H
