#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <QVector>
#include <QVector3D>

#include "../diskScanner.h"

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

      virtual void ScanDirectory();
      virtual void ParseScan() = 0;

      /**
       * @brief GetVertices
       * @return
       */
      QVector<QVector3D>& PopulateVertexBuffer();

      /**
       * @brief GetColors
       * @return
       */
      QVector<QVector3D>& PopulateColorBuffer();

      /**
       * @brief GetVertexCount returns the number of vertices currently in the model's vertex
       * buffer.
       *
       * @returns the number of vertices.
       */
      unsigned int GetVertexCount() const;

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

      QVector<QVector3D> m_visualizationVertices;
      QVector<QVector3D> m_visualizationColors;

      QVector<QVector3D> m_boundingBoxVertices;
};

#endif // VISUALIZATION_H
