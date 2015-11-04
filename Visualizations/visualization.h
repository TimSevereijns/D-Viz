#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <boost/optional.hpp>

#include <QVector>
#include <QVector3D>

#include <cstdint>
#include <memory>
#include <numeric>

#include "../tree.h"

#include "../DataStructs/vizNode.h"
#include "../Viewport/camera.h"

namespace Qt3D
{
   class QRay3D;
}

/**
 * @brief The VisualizationParameters struct represents the gamut of visualization parameters that
 * can be set to control when visualization updates occur, as well as what nodes get included.
 */
struct VisualizationParameters
{
   std::wstring rootDirectory;      ///< The path to the directory at the root of the visualization.

   std::uint64_t minimumFileSize;   ///< The minimum size a file should be before it shows up.

   bool forceNewScan;               ///< Whether a new scan of the rootDirectory should take place.
   bool onlyShowDirectories;        ///< Whether only directories should be shown.

   VisualizationParameters()
      : rootDirectory(L""),
        minimumFileSize(0),
        onlyShowDirectories(false),
        forceNewScan(true)
   {
   }
};

/**
 * @brief The Visualization class
 */
class Visualization
{
   public:
      explicit Visualization(const VisualizationParameters& parameters);
      virtual ~Visualization();

      static const double BLOCK_HEIGHT;
      static const double PADDING_RATIO;
      static const double MAX_PADDING;
      static const double ROOT_BLOCK_WIDTH;
      static const double ROOT_BLOCK_DEPTH;

      /**
       * Parses the specified directory scan into vertex and color data.
       *
       * @param[in/out] theTree         The unparsed scan results.
       */
      virtual void Parse(const std::shared_ptr<Tree<VizNode>>& theTree) = 0;

      /**
       * @brief UpdateBoundingBoxes will compute the minimum Axis-Aligned Bounding Boxes (AABB) for
       * each node in the tree. Each node's bounding box will not only minimally enclose the block
       * of the node to which it belongs, but also all descendants of the node in question.
       */
      virtual void UpdateBoundingBoxes();

      /**
       * @brief ComputeVertexAndColorData
       */
      void ComputeVertexAndColorData(const VisualizationParameters& parameters);

      /**
       * @brief GetVertexData
       *
       * @returns the vertices that represent the entire visualization.
       */
      QVector<QVector3D>& GetVertexData();

      /**
       * @brief GetColorData
       *
       * @returns the color data that is associated with the vertex data. @see GetVertexData
       */
      QVector<QVector3D>& GetColorData();

      /**
       * @brief FindNearestIntersection
       *
       * @param[in] ray             The ray to be shot into the scene.
       * @param[in] parameters      The current visualization parameters.
       *
       * @returns the closest node that the ray intersected with.
       */
      boost::optional<TreeNode<VizNode>> FindNearestIntersection(const Qt3D::QRay3D& ray,
         const VisualizationParameters& parameters) const;

      /**
       * @brief FindNearestIntersectionUsingAABB will attempt to identify the closest node in front
       * of the camera that the specified ray intersects with. This search operation is carried out
       * with aid of the minimum Axis-Aligned Bounding Boxes (AABB) that surround each node and its
       * descendants.
       *
       * @param[in] camera          The camera from which the ray originated.
       * @param[in] ray             The picking ray.
       * @param[in] parameters      @see VisualizationParameters. Used to prune disqualified nodes.
       *
       * @returns the node that was clicked on; boost::none otherwise.
       */
      boost::optional<TreeNode<VizNode>> FindNearestIntersectionUsingAABB(
         const Camera& camera,
         const Qt3D::QRay3D& ray,
         const VisualizationParameters& parameters) const;

      /**
       * @brief CreateBlockColors creates the vertex colors needed to color a single block.
       * 
       * @returns a vector of vertex colors.
       */
      static QVector<QVector3D> CreateFileColors();

      /**
       * @brief CreateBlockColors creates the vertex colors needed to color a single block.
       * 
       * @returns a vector of vertex colors.
       */
      static QVector<QVector3D> CreateDirectoryColors();

      /**
       * @brief SortNodes traverses the tree in a post-order fashion, sorting the children of each
       * node by their respective file sizes.
       *
       * @param[in/out] tree           The tree to be sorted.
       */
      static void SortNodes(Tree<VizNode>& tree);

   protected:
      std::shared_ptr<Tree<VizNode>> m_theTree; ///< @todo ...or should this be a weak_ptr?

      VisualizationParameters m_vizParameters;

      bool m_hasDataBeenParsed;

      QVector<QVector3D> m_visualizationVertices;
      QVector<QVector3D> m_visualizationColors;
};

#endif // VISUALIZATION_H
