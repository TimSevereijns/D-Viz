#ifndef BLOCK_H
#define BLOCK_H

#include <QVector>
#include <QVector3D>

/**
 * @brief The Block struct represents a single file or directory in the visualization. This
 * struct constains not only the basic dimensions of the block, but also all of its vertices
 * and color information. In addition to this, there is also some metadata to aid in the creation
 * of the squarified treemap.
 */
struct Block
{
   const static int VERTICES_PER_BLOCK = 60;

   QVector<QVector3D> m_vertices;
   QVector<QVector3D> m_colors;

   QVector3D m_nextRowOrigin; // Specific to the Squarified Treemap.

   double m_percentCovered;
   double m_width;
   double m_height;
   double m_depth;

   Block();

   /**
    * @brief Block creates the vertices needed to represent a single block. Each
    *        face consists of two triangles, and each vertex is followed by its corresponding
    *        normal. Since we are unlikely to see the bottom faces of the block, no vertices (or
    *        normals) wil be dedicated to visualizing it.
    *
    * @param bottomLeft             The bottom-left corner of the block under construction.
    * @param width                  The desired block width; width grows along positive x-axis.
    * @param height                 The desired block height; height grows along positive y-axis.
    * @param depth                  The desired block depth; depth grows along negative z-axis.
    *
    * @returns a vector of vertices.
    */
   Block(const QVector3D& bottomLeft, const double width, const double height, const double depth);

   /**
    * @brief IsDefined checks if width, height, and depth are all non-zero. It does not check
    * to see if the block is inverted (with respect to where the normals of opposing faces point);
    * call IsValid() to perform that check.
    *
    * @returns true if the block is properly defined.
    */
   bool IsDefined() const;

   /**
    * @brief IsValid performs a quick check of Cartesian X-axis coordinates to determine if the
    * block is in a valid state.
    *
    * @returns true if the block is defined and the left face is indeed to the left of the right
    * face; false otherwise.
    */
   bool IsValid() const;

   /**
    * @brief GetOriginPlusHeight.
    * @returns the coordinates of the block's origin offset by the height of the block.
    */
   QVector3D GetOriginPlusHeight() const;
};
#endif // BLOCK_H
