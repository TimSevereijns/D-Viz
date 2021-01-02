#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <algorithm>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Factories/modelFactory.h"
#include "Factories/modelFactoryInterface.h"
#include "Factories/viewFactory.h"
#include "Factories/viewFactoryInterface.h"
#include "Model/Monitor/fileChangeNotification.h"
#include "Model/Scanner/driveScanner.h"
#include "Model/vizBlock.h"
#include "Settings/nodePainter.h"
#include "Settings/persistentSettings.h"
#include "Settings/sessionSettings.h"
#include "View/Scene/light.h"
#include "View/mainWindow.h"
#include "constants.h"

#if defined(Q_OS_WIN)
#include "Model/Monitor/windowsFileMonitor.h"
#elif defined(Q_OS_LINUX)
#include "Model/Monitor/linuxFileMonitor.h"
#endif // Q_OS_LINUX

#include <Tree/Tree.hpp>

#include <QVector3D>

class FileEvent;
class GLCanvas;
class ScanningProgress;

class Controller : public QObject
{
    Q_OBJECT

  public:
#if defined(Q_OS_WIN)
    using FileSystemMonitor = WindowsFileMonitor;
#elif defined(Q_OS_LINUX)
    using FileSystemMonitor = LinuxFileMonitor;
#endif // Q_OS_LINUX

    Controller(ViewFactoryInterface& viewFactory, ModelFactoryInterface& modelFactory);

    /**
     * @brief Starts the UI.
     */
    void LaunchUI();

    /**
     * @brief Enables or disables the monitoring of the filesystem.
     *
     * @param[in] shouldEnable      The new state of the monitor.
     */
    void MonitorFileSystem(bool shouldEnable);

    /**
     * @brief Scans the drive.
     *
     * @param[in] parameters      Drive scanning parameters; where to start, et cetera.
     */
    void ScanDrive(const Settings::VisualizationParameters& parameters);

    /**
     * @brief Stops any active drive scanner.
     */
    void StopScanning();

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
     * @param[in] extension       The extension to be highlighted.
     * @param[in] callback        UI callback to highlight matching nodes on the canvas.
     */
    void HighlightAllMatchingExtensions(
        const std::wstring& extension,
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
     * @brief Uses the passed in ray to select the nearest node from the perspective of the camera.
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
     * @brief Computes the absolute file path of the selected node by traveling up tree.
     *
     * @param[in] node            The selected node.
     *
     * @returns The absolute file path.
     */
    static std::filesystem::path NodeToFilePath(const Tree<VizBlock>::Node& node);

    /**
     * @brief Prints selection details to the main window's status bar.
     */
    void DisplayHighlightDetails();

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
     * @param[in] progress        See ScanningProgress
     */
    void SaveScanMetadata(const ScanningProgress& progress);

    /**
     * @returns Settings that persist between application usages.
     */
    Settings::PersistentSettings& GetPersistentSettings();

    /**
     * @overload
     */
    const Settings::PersistentSettings& GetPersistentSettings() const;

    /**
     * @returns Settings that expire when the application ends.
     */
    Settings::SessionSettings& GetSessionSettings();

    /**
     * @overload
     */
    const Settings::SessionSettings& GetSessionSettings() const;

    /**
     * @returns Settings needed to determine node colors.
     */
    Settings::NodePainter& GetNodePainter();

    /**
     * @overload
     */
    const Settings::NodePainter& GetNodePainter() const;

    /**
     * @returns The filesystem path belonging to the root of the visualization.
     */
    std::filesystem::path GetRootPath() const;

    /**
     * @brief Starts monitoring the file system. This is only possible after the first scan.
     */
    void MonitorFileSystem();

    /**
     * @returns True if filesystem monitoring is enabled; false otherwise.
     */
    bool IsFileSystemBeingMonitored() const;

    /**
     * @brief Fetches oldest, unprocessed file system change notification.
     */
    std::optional<FileEvent> FetchNextFileModification();

    /**
     * @brief Determines the color a given node should be.
     *
     * @param[in] node              The node whose color is to be determined.
     *
     * @returns The appropriate color.
     */
    QVector3D DetermineNodeColor(const Tree<VizBlock>::Node& node) const;

    /**
     * @brief Records the temporary color of a node.
     *
     * This function should only be called to register colors that are not the "normal" color for a
     * particular node, given the current visualization settings. Instead, this function is to be
     * used to register the fact that a node might be highlight as part of a search, for instance.
     *
     * @param[in] node              The node whose color is to be registered.
     * @param[in] color             The color that the node is to be assigned.
     */
    void RegisterNodeColor(const Tree<VizBlock>::Node& node, const QVector3D& color);

  signals:

    /**
     * @brief Signals that the drive scanning has finished.
     */
    void FinishedScanning();

  private:
    template <typename NodeSelectorType>
    void ProcessHighlightedNodes(
        const NodeSelectorType& nodeSelector,
        const std::function<void(std::vector<const Tree<VizBlock>::Node*>&)>& callback);

    void ReportProgressToStatusBar(const ScanningProgress& progress);

    template <typename ButtonType>
    void ReportProgressToTaskbar(ButtonType& button, const ScanningProgress& progress);

    void OnScanComplete(
        const Settings::VisualizationParameters& parameters, const ScanningProgress& progress,
        const std::shared_ptr<Tree<VizBlock>>& scanningResults);

    ViewFactoryInterface& m_viewFactory;
    ModelFactoryInterface& m_modelFactory;

    bool m_allowInteractionWithModel{ false };

    Settings::PersistentSettings m_persistentSettings;
    Settings::SessionSettings m_sessionSettings;
    Settings::NodePainter m_nodePainter;

    std::shared_ptr<BaseView> m_view{ nullptr };
    std::shared_ptr<BaseModel> m_model{ nullptr };

    DriveScanner m_scanner;

    std::shared_ptr<BaseTaskbarButton> m_taskbarProgress;

    std::uint64_t m_occupiedDiskSpace{ 0u };

    // @todo Move this onto the model.
    std::unordered_map<std::uintptr_t, QVector3D> m_nodeColorMap;
};

#endif // CONTROLLER_H
