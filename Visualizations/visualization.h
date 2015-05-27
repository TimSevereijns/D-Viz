#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <QVector>
#include <QVector3D>

#include <cstdint>
#include <numeric>

#include "../diskScanner.h"

/**
 * @brief The ParsingOptions struct represents the gamut of parsing options
 * available. This includes such things as to whether to show only directory,
 * whether to filter out files of a certain size, or whether to force a new
 * disk scan.
 */
struct ParsingOptions
{
   bool showDirectoriesOnly;
   bool forceNewScan;
   std::uint64_t fileSizeMinimum;

   ParsingOptions()
      : showDirectoriesOnly(false),
        forceNewScan(false),
        fileSizeMinimum(std::numeric_limits<std::uintmax_t>::max())
   {
   }
};

/**
 * @brief The Visualization class
 */
class Visualization
{
   public:
      explicit Visualization(const std::wstring& rawPath);
      virtual ~Visualization();

      static const double BLOCK_HEIGHT;
      static const double PADDING_RATIO;
      static const double MAX_PADDING;
      static const double ROOT_BLOCK_WIDTH;
      static const double ROOT_BLOCK_DEPTH;

      /**
       * @brief ScanDirectory stars a scan of the system, starting at the location specified
       * through the constructor.
       * 
       * @param[in] statusBarUpdater         The function to be called once progress updates.
       */
      virtual void ScanDirectory(const std::function<void (const std::uintmax_t)> statusBarUpdater);
      
      /**
       * A pure virtual function that indicates how the scan is to be parsed and interpreted.
       */
      virtual void ParseScan() = 0;

      /**
       * @brief PopulateVertexBuffer flushes the existing VBO and loads the newly
       * parsed vertices.
       * 
       * @param[in] options            The options that specify how the scan is to
       *                               be parsed and interpreted.
       * 
       * @returns a vector of vertices.
       */
      QVector<QVector3D>& PopulateVertexBuffer(const ParsingOptions& options);

      /**
       * @brief PopulateColorBuffer flushes the existing color buffer and reloads
       * it with the data from the latest parse.
       * 
       * @param[in] options            The options that specify how the scan is to
       *                               be parse and interpreted.
       * 
       * @returns a vector of colors data per vertex.
       */
      QVector<QVector3D>& PopulateColorBuffer(const ParsingOptions& options);

      /**
       * @brief GetVertexCount returns the number of vertices currently in the model's vertex
       * buffer.
       *
       * @returns the number of vertices.
       */
      unsigned int GetVertexCount() const;

      /**
       * @brief HasScanBeenPerformed indicates whether the scan has been performed.
       * 
       * @returns true if the scan has been performed; false otherwise.
       */
      bool HasScanBeenPerformed() const;

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
      DiskScanner m_diskScanner;

      bool m_hasDataBeenParsed;
      bool m_hasScanBeenPerformed;

      QVector<QVector3D> m_visualizationVertices;
      QVector<QVector3D> m_visualizationColors;

      QVector<QVector3D> m_boundingBoxVertices;
};

#endif // VISUALIZATION_H
