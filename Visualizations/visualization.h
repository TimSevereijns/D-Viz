#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <QVector>
#include <QVector3D>

#include <cstdint>
#include <memory>
#include <numeric>

#include "../tree.h"

#include "../DataStructs/vizNode.h"

/**
 * @brief The ParsingOptions struct represents the gamut of visualization parameters that can be
 * adjusted.
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
       * Parses the specified directory scan into the vertex and color buffers.
       */
      virtual void Parse(const std::shared_ptr<Tree<VizNode>>& theTree) = 0;

      /**
       * @brief PopulateVertexAndColorBuffers
       */
      void PopulateVertexAndColorBuffers(const VisualizationParameters& parameters);

      /**
       * @brief GetVertexBuffer
       * @return
       */
      QVector<QVector3D>& GetVertexBuffer();

      /**
       * @brief GetColorBuffer
       * @return
       */
      QVector<QVector3D>& GetColorBuffer();

      /**
       * @brief ComputeNearestIntersection
       * @returns the distance to the nearest intersection point in the visualization.
       */
      double ComputeNearestIntersection(std::pair<QVector3D, QVector3D> ray) const;

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
       * @brief SortNodes traverses the tree in a post-order fashion, sorting the children of each node
       * by their respective file sizes.
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
