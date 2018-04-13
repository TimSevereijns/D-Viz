#ifndef VISUALIZATIONMODEL_H
#define VISUALIZATIONMODEL_H

#include <boost/optional.hpp>

#include <QVector>
#include <QVector3D>

#include <cstdint>
#include <memory>
#include <numeric>
#include <thread>

#include <Tree/Tree.hpp>

#include "../DataStructs/vizBlock.h"
#include "../Settings/settings.h"
#include "../Viewport/camera.h"

struct TreemapMetadata
{
   std::uintmax_t FileCount;
   std::uintmax_t DirectoryCount;
   std::uintmax_t TotalBytes;
};

/**
 * @brief Base class for the visualization model.
 */
class VisualizationModel
{
   public:

      VisualizationModel() = default;

      virtual ~VisualizationModel() = default;

      VisualizationModel(const VisualizationModel&) = delete;
      VisualizationModel& operator=(const VisualizationModel&) = delete;

      static const double PADDING_RATIO;
      static const double MAX_PADDING;

      static const float BLOCK_HEIGHT;
      static const float ROOT_BLOCK_WIDTH;
      static const float ROOT_BLOCK_DEPTH;

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
       * @param[in] parameters      @see VisualizationParameters. Used to prune disqualified nodes.
       *
       * @returns A pointer to the TreeNode that was clicked on, and nullptr if no intersection
       * exists.
       */
      Tree<VizBlock>::Node* FindNearestIntersection(
         const Camera& camera,
         const Qt3DRender::RayCasting::QRay3D& ray,
         const Settings::VisualizationParameters& parameters) const;

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
        * @param[in] data
        */
      void SetTreemapMetadata(TreemapMetadata&& data);

      /**
       * @returns Metadata about the visualization.
       */
      TreemapMetadata GetTreemapMetadata();

      /**
       * @brief Highlights all ancestors of the given node.
       *
       * @param[in] node            The starting node.
       */
      void HighlightAncestors(const Tree<VizBlock>::Node& node);

      /**
       * @brief Highlights all descendents of the given node.
       *
       * @param[in] node            The starting node.
       * @param parameters          @see VisualizationParameters. Used to prune disqualified nodes.
       */
      void HighlightDescendants(
         const Tree<VizBlock>::Node& node,
         Settings::VisualizationParameters parameters);

      /**
       * @brief Highlights all nodes that match the sample node's extension.
       *
       * @param[in] sampleNode      The node whose extension should be highlighted.
       * @param parameters
       */
      void HighlightMatchingFileExtension(
         const Tree<VizBlock>::Node& sampleNode,
         Settings::VisualizationParameters parameters);

      /**
       * @brief Highlights all nodes that match the search query, given the search parameters.
       *
       * @param[in] searchQuery              The raw search query.
       * @param[in] parameters               @see VisualizationParameters. Used to prune
       *                                     disqualified nodes.
       * @param[in] shouldSearchFiles        Pass in true to search files.
       * @param[in] shouldSearchDirectories  Pass in true to search directories.
       */
      void HighlightMatchingFileName(
         const std::wstring& searchQuery,
         Settings::VisualizationParameters parameters,
         bool shouldSearchFiles,
         bool shouldSearchDirectories);

      /**
       * @brief Starts monitoring the file system for changes.
       *
       * @todo Pass in callback to handle changes.
       */
      void StartFileSystemMonitor();

      /**
       * @brief SortNodes traverses the tree in a post-order fashion, sorting the children of each
       * node by their respective file sizes.
       *
       * @param[in, out] tree           The tree to be sorted.
       */
      static void SortNodes(Tree<VizBlock>& tree);

   protected:

      /**
       * @brief Performs the actual monitoring of the filesystem.
       */
      void MonitorFileSystem();

      // @note The tree is stored in a shared pointer so that it can be passed through the Qt
      // signaling framework; any type passed through it needs to be copy-constructible.
      std::shared_ptr<Tree<VizBlock>> m_fileTree{ nullptr };

      std::vector<const Tree<VizBlock>::Node*> m_highlightedNodes;

      const Tree<VizBlock>::Node* m_selectedNode{ nullptr };

      TreemapMetadata m_metadata{ 0, 0, 0 };

      bool m_hasDataBeenParsed{ false };

      //std::thread m_fileMonitor;
};

#endif // VISUALIZATIONMODEL_H
