#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <boost/optional.hpp>

#include <QVector>
#include <QVector3D>

#include <cstdint>
#include <memory>
#include <numeric>

#include "../ThirdParty/Tree.hpp"

#include "../DataStructs/vizNode.h"
#include "../Viewport/camera.h"

/**
 * @brief The VisualizationParameters struct represents the gamut of visualization parameters that
 * can be set to control when visualization updates occur, as well as what nodes get included.
 */
struct VisualizationParameters
{
   std::wstring rootDirectory{ L"" };  ///< The path to the root directory

   std::uint64_t minimumFileSize{ 0 }; ///< The minimum size a file should be before it shows up.

   bool forceNewScan{ true };          ///< Whether a new scan should take place.
   bool onlyShowDirectories{ false };  ///< Whether only directories should be shown.
   bool useDirectoryGradient{ false }; ///< Whether to use gradient coloring for the directories.
};

/**
 * @brief Base visualization class.
 */
class VisualizationModel
{
   public:
      explicit VisualizationModel(const VisualizationParameters& parameters);

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
      virtual void Parse(const std::shared_ptr<Tree<VizNode>>& theTree) = 0;

      /**
       * @brief Updates the minimum Axis-Aligned Bounding Boxes (AABB) for each node in the tree.
       *
       * Each node's bounding box will not only minimally enclose the block
       * of the node to which it belongs, but also all descendants of the node in question.
       */
      virtual void UpdateBoundingBoxes();

      /**
       * @brief Attempts to identify the closest node in front of the camera that the specified ray
       * intersects with.
       *
       * This search operation is carried out with aid of the minimum Axis-Aligned Bounding Boxes
       * (AABB) that surround each node and its descendants.
       *
       * @param[in] camera          The camera from which the ray originated.
       * @param[in] ray             The picking ray.
       * @param[in] parameters      @see VisualizationParameters. Used to prune disqualified nodes.
       *
       * @returns A pointer to the TreeNode that was clicked on, and nullptr if no intersection
       * exists.
       */
      TreeNode<VizNode>* FindNearestIntersection(
         const Camera& camera,
         const Qt3DCore::QRay3D& ray,
         const VisualizationParameters& parameters) const;

      /**
       * @brief GetTree
       * @return
       */
      Tree<VizNode>& GetTree();

      /**
       * @brief SortNodes traverses the tree in a post-order fashion, sorting the children of each
       * node by their respective file sizes.
       *
       * @param[in, out] tree           The tree to be sorted.
       */
      static void SortNodes(Tree<VizNode>& tree);

   protected:
      std::shared_ptr<Tree<VizNode>> m_theTree{ nullptr };

      bool m_hasDataBeenParsed{ false };

      VisualizationParameters m_vizParameters;
};

#endif // VISUALIZATION_H
