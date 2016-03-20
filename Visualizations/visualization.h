#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <boost/optional.hpp>

#include <QVector>
#include <QVector3D>

#include <cstdint>
#include <memory>
#include <numeric>

//#include "../tree.h"
#include "../ThirdParty/Tree.hpp"

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
 * @brief Base visualization class.
 */
class Visualization
{
   public:
      explicit Visualization(const VisualizationParameters& parameters);
      virtual ~Visualization();

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
       * @brief Will update the minimum Axis-Aligned Bounding Boxes (AABB) for each node in the
       * tree.
       *
       * Each node's bounding box will not only minimally enclose the block
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
       * @returns The vertices that represent the entire visualization.
       */
      QVector<QVector3D>& GetVertexData();

      /**
       * @brief GetColorData
       *
       * @returns The color data that is associated with the vertex data. @see GetVertexData
       */
      QVector<QVector3D>& GetColorData();

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
       * @returns Pointer to the TreeNode that was clicked on, and nullptr if no intersection
       * exists.
       */
      TreeNode<VizNode>* FindNearestIntersection(
         const Camera& camera,
         const Qt3D::QRay3D& ray,
         const VisualizationParameters& parameters) const;

      /**
       * @brief Creates the vertex colors needed to color a single block.
       *
       * @returns A vector of colors.
       */
      static QVector<QVector3D> CreateFileColors();

      /**
       * @brief Creates the vertex colors needed to color a single block.
       *
       * @returns A vector of colors.
       */
      static QVector<QVector3D> CreateDirectoryColors();

      /**
       * @brief Create the vertex colors needed to color the selected file.
       *
       * @return A vector of colors.
       */
      static QVector<QVector3D> CreateHighlightColors();

      /**
       * @brief SortNodes traverses the tree in a post-order fashion, sorting the children of each
       * node by their respective file sizes.
       *
       * @param[in, out] tree           The tree to be sorted.
       */
      static void SortNodes(Tree<VizNode>& tree);

   protected:
      std::shared_ptr<Tree<VizNode>> m_theTree;

      VisualizationParameters m_vizParameters;

      bool m_hasDataBeenParsed;

      QVector<QVector3D> m_visualizationVertices;
      QVector<QVector3D> m_visualizationColors;
};

#endif // VISUALIZATION_H
