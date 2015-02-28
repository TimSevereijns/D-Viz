#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <QVector>
#include <QVector3D>

#include "diskScanner.h"

class Visualization
{
   public:
      virtual ~Visualization() {}

      virtual void ParseDirectoryScan() = 0;

   protected:
      DiskScanner m_diskScanner;

      QVector<QVector3D> m_visualizationVertices;
      QVector<QVector3D> m_visualizationColors;
};

#endif // VISUALIZATION_H
