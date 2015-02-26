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
       * @brief CreateBlockVertices creates the vertices needed to represent a single block. Each
       *        face consists of two triangles, and each vertex is followed by its corresponding
       *        normal.
       * @param bottomLeft             The bottom-left corner of the block under construction.
       * @param width                  The desired block width; width grows along positive x-axis.
       * @param height                 The desired block height; height grows along positive y-axis.
       * @param depth                  The desired block depth; depth grows along negative z-axis.
       * @returns a vector of vertices.
       */
      static QVector<QVector3D> CreateBlockVertices(const QVector3D& bottomLeft, const float width,
         const float height, const float depth);

      /**
       * @brief CreateBlockColors creates the vertex colors needed to color a single block.
       * @returns a vector of vertex colors.
       */
      static QVector<QVector3D> CreateBlockColors();

   private:
      DiskScanner m_diskScanner;
};

#endif // TREEMAP_H
