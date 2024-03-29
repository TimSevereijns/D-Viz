#ifndef VISUALIZATIONMODEL_H
#define VISUALIZATIONMODEL_H

#include <QVector3D>
#include <QVector>

#include <cstdint>
#include <memory>
#include <numeric>
#include <optional>
#include <thread>
#include <unordered_map>

#include <Tree/Tree.hpp>

#include "Model/Monitor/fileChangeNotification.h"
#include "Model/Monitor/fileSystemObserver.h"
#include "Model/vizBlock.h"
#include "Settings/settings.h"
#include "Settings/visualizationOptions.h"
#include "Utilities/threadSafeQueue.h"
#include "View/Viewport/camera.h"

struct TreemapMetadata
{
    std::uintmax_t FileCount = 0;
    std::uintmax_t DirectoryCount = 0;
    std::uintmax_t TotalBytes = 0;
};

enum SearchFlags : int
{
    SearchFiles = 1,
    SearchDirectories = 2,
    UseRegex = 4
};

constexpr inline SearchFlags operator|(SearchFlags lhs, SearchFlags rhs)
{
    return static_cast<SearchFlags>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

constexpr inline SearchFlags& operator|=(SearchFlags& lhs, SearchFlags rhs)
{
    return lhs = lhs | rhs;
}

/**
 * @brief Base class for the visualization model.
 */
class BaseModel
{
  public:
    BaseModel(std::unique_ptr<FileMonitorBase> fileMonitor, const std::filesystem::path& path);

    virtual ~BaseModel() noexcept;

    BaseModel(const BaseModel&) = delete;
    BaseModel& operator=(const BaseModel&) = delete;

    BaseModel(BaseModel&&) = delete;
    BaseModel& operator=(BaseModel&&) = delete;

    /**
     * @brief Parses the specified directory scan into vertex and color data.
     *
     * @param[in, out] theTree    The unparsed scan results.
     */
    virtual void Parse(const std::shared_ptr<Tree<VizBlock>>& theTree) = 0;

    /**
     * @brief Updates the minimum Axis-Aligned Bounding Boxes (AABB) for each node in the tree.
     *
     * Each node's bounding box will not only minimally enclose the block
     * of the node to which it belongs, but also all descendants of the node in question.
     */
    virtual void UpdateBoundingBoxes();

    /**
     * @brief Identifies the closest node in front of the camera that the specified ray intersects
     * with.
     *
     * This search operation is carried out with aid of the minimum Axis-Aligned Bounding Boxes
     * (AABB) that surround each node and its descendants.
     *
     * @todo Remove the camera from the parameter list; just pass in a point..
     *
     * @param[in] camera          The camera from which the ray originated.
     * @param[in] ray             The picking ray.
     * @param[in] options         Used to prune disqualified nodes. @see VisualizationOptions.
     *
     * @returns A pointer to the TreeNode that was clicked on, and nullptr if no intersection
     * exists.
     */
    Tree<VizBlock>::Node* FindNearestIntersection(
        const Camera& camera, const Ray& ray, const Settings::VisualizationOptions& options) const;

    /**
     * @returns A reference to the directory tree.
     */
    Tree<VizBlock>& GetTree();

    /**
     * @overload
     */
    const Tree<VizBlock>& GetTree() const;

    /**
     * @returns The currently highlighted nodes.
     */
    const std::vector<const Tree<VizBlock>::Node*>& GetHighlightedNodes() const;

    /**
     * @overload
     */
    std::vector<const Tree<VizBlock>::Node*>& GetHighlightedNodes();

    /**
     * @brief Clears the currently highlighted nodes.
     */
    void ClearHighlightedNodes();

    /**
     * @brief Selects the supplied node.
     */
    void SelectNode(const Tree<VizBlock>::Node& node);

    /**
     * @returns The currently selected node.
     */
    const Tree<VizBlock>::Node* GetSelectedNode();

    /**
     * @brief Clears the currently selected node.
     */
    void ClearSelectedNode();

    /**
     * @brief Sets treemap metadata.
     *
     * @param[in] data              The data to be saved.
     */
    void SetTreemapMetadata(TreemapMetadata&& data);

    /**
     * @returns Metadata about the visualization.
     */
    TreemapMetadata GetTreemapMetadata();

    /**
     * @brief HighlightNode
     * @param[in] node              The node to be highlighted.
     */
    void HighlightNode(const Tree<VizBlock>::Node* const node);

    /**
     * @brief Highlights all ancestors of the given node.
     *
     * @param[in] node            The starting node.
     */
    void HighlightAncestors(const Tree<VizBlock>::Node& node);

    /**
     * @brief Highlights all descendents of the given node.
     *
     * @param[in] root            The starting node.
     * @param options             Used to prune disqualified nodes. @see VisualizationOptions.
     */
    void HighlightDescendants(
        const Tree<VizBlock>::Node& root, const Settings::VisualizationOptions& options);

    /**
     * @brief Highlights all nodes that match the sample node's extension.
     *
     * @param[in] extension       The extension that should be highlighted.
     * @param[in] options         Used to prune disqualified nodes. @see VisualizationOptions.
     */
    void HighlightMatchingFileExtensions(
        const std::string& extension, const Settings::VisualizationOptions& options);

    /**
     * @brief Highlights all nodes that match the search query, given the search options.
     *
     * @param[in] searchQuery     The raw search query.
     * @param[in] options         Used to prune disqualified nodes. @see VisualizationOptions.
     * @param[in] flags           Bitmask of search options.
     */
    void HighlightMatchingFileNames(
        const std::string& searchQuery, const Settings::VisualizationOptions& options,
        SearchFlags flags);

    /**
     * @brief Starts monitoring the file system for changes.
     *
     * Once file system monitoring has been enabled, call the `FetchNextFileSystemChange()`
     * function to retrieve the next available notification.
     */
    void StartMonitoringFileSystem();

    /**
     * @brief Stops monitoring the file system for changes.
     */
    void StopMonitoringFileSystem() noexcept;

    /**
     * @returns True if the file system monitor is turned on.
     */
    bool IsFileSystemBeingMonitored() const;

    /**
     * @brief Returns the next visual changes in need of processing.
     *
     * @returns The metadata on the next available file to have changed since the visualization
     * was last refreshed.
     */
    std::optional<FileEvent> FetchNextVisualChange();

    /**
     * @brief Returns the next model changes in need of processing.
     *
     * @returns The metadata on the next available file to have changed since the visualization
     * was last refreshed.
     */
    std::optional<FileEvent> FetchNextModelChange();

    /**
     * @returns The root path for the current visualization. If no visualization has been loaded,
     * a default constructor path object will be returned.
     */
    std::filesystem::path GetRootPath() const;

    /**
     * @brief Blocks thread until the next filesystem model change has been observed.
     */
    void WaitForNextModelChange();

    /**
     * @brief Applies all pending visualization updates to the model.
     */
    void RefreshTreemap();

    /**
     * @brief SortNodes traverses the tree in a post-order fashion, sorting the children of each
     * node by their respective file sizes.
     *
     * @param[in, out] tree           The tree to be sorted.
     */
    static void SortNodes(Tree<VizBlock>& tree);

  protected:
    void UpdateAffectedNodes(const FileEvent& notification);

    void UpdateAncestorSizes(Tree<VizBlock>::Node* node);

    void ProcessChanges();

    void OnFileCreation(const FileEvent& notification);

    void OnFileDeletion(const FileEvent& notification);

    void OnFileModification(const FileEvent& notification);

    void OnFileNameChange(const FileEvent& notification);

    std::filesystem::path m_rootPath;

    // The tree is stored in a shared pointer so that it can be passed through the Qt
    // signaling framework; any type passed through it needs to be copy-constructible.
    std::shared_ptr<Tree<VizBlock>> m_fileTree; ///< @todo Does this need a mutex?

    // While only a single node can be "selected" at any given time, multiple nodes can be
    // "highlighted." This vector tracks those highlighted nodes.
    std::vector<const Tree<VizBlock>::Node*> m_highlightedNodes;

    // The one and only "selected" node, should one exist.
    const Tree<VizBlock>::Node* m_selectedNode = nullptr;

    TreemapMetadata m_metadata{ 0, 0, 0 };

    bool m_hasDataBeenParsed = false;

    FileSystemObserver m_fileSystemObserver;

    // This queue contains raw notifications of file system changes that still need to be
    // parsed and the turned into tree node change notifications.
    ThreadSafeQueue<FileEvent> m_fileEvents;

    // This queue contains pending tree node change notifications. These notifications
    // still need to be retrieved by the view so that the UI can be updated to visually represent
    // filesystem activity.
    ThreadSafeQueue<FileEvent> m_pendingVisualUpdates;

    // This queue contains pending changes that will need to be applied to the treemap once the user
    // refreshes the visualization to reflect filesystem changes. These notifications are best
    // processed in the order in which they occurred.
    ThreadSafeQueue<FileEvent> m_pendingModelUpdates;

    std::thread m_fileSystemNotificationProcessor;

    std::condition_variable m_eventNotificationReady;
    std::mutex m_eventNotificationMutex;

    std::atomic_bool m_shouldKeepProcessingNotifications = true;

  private:
    void PerformRegexSearch(
        const std::string& searchQuery, const Settings::VisualizationOptions& options,
        SearchFlags flags);

    void PerformNormalSearch(
        const std::string& searchQuery, const Settings::VisualizationOptions& options,
        SearchFlags flags);
};

#endif // VISUALIZATIONMODEL_H
