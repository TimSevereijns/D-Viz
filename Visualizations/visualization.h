#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <QVector>
#include <QVector3D>

#include <cstdint>
#include <numeric>

#include "../diskScanner.h"

/**
 * @brief The ParsingOptions struct
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

      virtual void ScanDirectory(std::function<void (const std::uintmax_t)> statusBarUpdater);
      virtual void ParseScan() = 0;

      /**
       * @brief PopulateVertexBuffer
       * @param options
       * @return
       */
      QVector<QVector3D>& PopulateVertexBuffer(const ParsingOptions& options);

      /**
       * @brief PopulateColorBuffer
       * @param options
       * @return
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
       * @brief HasScanBeenPerformed
       * @return
       */
      bool HasScanBeenPerformed() const;

      /**
       * @brief CreateBlockColors creates the vertex colors needed to color a single block.
       * @returns a vector of vertex colors.
       */
      static QVector<QVector3D> CreateBlockColors();

      /**
       * @brief CreateBlockColors creates the vertex colors needed to color a single block.
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
