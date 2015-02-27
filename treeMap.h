#ifndef TREEMAP_H
#define TREEMAP_H

#include <string>

#include <QVector>
#include <QVector3D>

#include "diskScanner.h"

class TreeMap
{
   public:
      explicit TreeMap();

      /**
       * @brief TreeMap creates a new instance of the Tree Map.
       * @param rawRootNodePath     The disk location at which to start scanning.
       */
      explicit TreeMap(const std::wstring& rawRootNodePath);
      ~TreeMap();

      /**
       * @brief ParseDirectoryTree
       */
      Tree<VizNode>& ParseDirectoryTree();

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

   private:
      DiskScanner m_diskScanner;
};

#endif // TREEMAP_H
