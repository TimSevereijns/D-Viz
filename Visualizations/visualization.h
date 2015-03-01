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

      virtual void ScanDirectory();
      virtual void ParseScan() = 0;

      QVector<QVector3D>& GetVertices();
      QVector<QVector3D>& GetColors();

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
};

#endif // VISUALIZATION_H
