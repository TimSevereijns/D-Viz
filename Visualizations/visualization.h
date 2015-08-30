#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <QVector>
#include <QVector3D>

#include <cstdint>
#include <memory>
#include <numeric>

#include "../tree.h"
#include "../diskScanner.h"

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
       * @brief PopulateVertexBuffer flushes the existing VBO and loads the newly
       * parsed vertices.
       * 
       * @param[in] parameters         @see VisualizationParameters
       * 
       * @returns a vector of vertices.
       */
      QVector<QVector3D>& PopulateVertexBuffer(const VisualizationParameters& parameters);

      /**
       * @brief PopulateColorBuffer flushes the existing color buffer and reloads
       * it with the data from the latest parse.
       * 
       * @param[in] options            The options that specify how the scan is to
       *                               be parse and interpreted.
       * 
       * @returns a vector of colors data per vertex.
       */
      QVector<QVector3D>& PopulateColorBuffer(const VisualizationParameters& options);

      /**
       * @brief GetVertexCount returns the number of vertices currently in the model's vertex
       * buffer.
       *
       * @returns the number of vertices.
       */
      unsigned int GetVertexCount() const;

      /**
       * @brief CreateBlockColors creates the vertex colors needed to color a single block.
       * 
       * @returns a vector of vertex colors.
       */
      static QVector<QVector3D> CreateBlockColors();

      /**
       * @brief CreateBlockColors creates the vertex colors needed to color a single block.
       * 
       * @returns a vector of vertex colors.
       */
      static QVector<QVector3D> CreateDirectoryColors();

   protected:
      std::shared_ptr<Tree<VizNode>> m_theTree; ///< @todo ...or should this be a weak_ptr?

      VisualizationParameters m_vizParameters;

      bool m_hasDataBeenParsed;

      QVector<QVector3D> m_visualizationVertices;
      QVector<QVector3D> m_visualizationColors;

      QVector<QVector3D> m_boundingBoxVertices;
};

#endif // VISUALIZATION_H
