#ifndef BLOCK_H
#define BLOCK_H

#include "doublePoint3d.h"

#include <QVector>
#include <QVector3D>

#include <iterator>

/**
 * @brief The BlockFace struct represents the vertices (and their normals) for a a single face of
 * the Block struct.
 *
 * @see Block
 */
struct BlockFace
{
   /// @note the enum value order is dictated by the face insertion order into Block::m_blockFaces.
   enum struct Side
   {
      FRONT = 0,     ///< Normal points towards +Z in OpenGL.
      RIGHT,         ///< Normal points towards +X in OpenGL.
      BACK,          ///< Normal points towards -Z in OpenGL.
      LEFT,          ///< Normal points towards -X in OpenGL.
      TOP            ///< Normal points towards +Y in OpenGL.
   };

   explicit BlockFace()
      : side(Side::FRONT)
   {
   }

   explicit BlockFace(const QVector<QVector3D>& vertices, const Side side)
      : vertices(vertices),
        side(side)
   {
   }

   QVector<QVector3D> vertices;
   Side side;
};

/**
 * @brief The Block struct represents a single file or directory in the visualization. This
 * struct constains not only the basic dimensions of the block, but also all of its vertices
 * and color information. In addition to this, there is also some metadata to aid in the creation
 * of the squarified treemap.
 */
struct Block
{
   class FaceIterator;
   friend class FaceIterator;

   const static int FACES_PER_BLOCK = 5;
   const static int VERTICES_PER_BLOCK = 60;

   QVector<QVector3D> colors;

   QVector<BlockFace> blockFaces;

   DoublePoint3D origin;
   DoublePoint3D nextRowOrigin; ///< Specific to the Squarified Treemap.

   double percentCovered;
   double width;
   double height;
   double depth;

   explicit Block();

   /**
    * @brief Block creates the vertices needed to represent a single block. Each face consists of
    * two triangles, and each vertex is followed by its corresponding normal. Since we are unlikely
    * to see the bottom faces of the block, no vertices (or normals) wil be dedicated to visualizing
    * it.
    *
    * @param[in] origin             The bottom-left corner of the block under construction.
    * @param[in] width              The desired block width; width grows along positive x-axis.
    * @param[in] height             The desired block height; height grows along positive y-axis.
    * @param[in] depth              The desired block depth; depth grows along negative z-axis.
    */
   explicit Block(
      const DoublePoint3D& origin,
      const double width,
      const double height,
      const double depth);

   /**
    * @brief begin
    *
    * @returns an iterator to the "first" face of the block.
    */
   FaceIterator begin() const;

   /**
    * @brief end
    *
    * @returns an iterator past the "last" face of the block.
    */
   FaceIterator end() const;

   /**
    * @brief IsDefined checks if width, height, and depth are all non-zero. It does not check
    * to see if the block is inverted (with respect to where the normals of opposing faces point);
    * call IsValid() to perform that check.
    *
    * @returns true if the block is properly defined.
    */
   bool HasVolume() const;

   /**
    * @brief IsValid performs a quick check of Cartesian X-axis coordinates to determine if the
    * block is in a valid state.
    *
    * @returns true if the block is defined and the left face is indeed to the left of the right
    * face; false otherwise.
    */
   bool IsNotInverted() const;

   /**
    * @brief GetNextChildOrigin will return the location at which to start the laying out immediate
    * descendants.
    *
    * @returns the coordinates of the block's origin offset by the height of the block.
    */
   DoublePoint3D GetNextChildOrigin() const;

   /**
    * @brief The FaceInterator class provides a const iterator over all the faces that make up a
    * block.
    */
   class FaceIterator
   {
      public:
         typedef BlockFace                         value_type;
         typedef BlockFace*                        pointer;
         typedef BlockFace&                        reference;
         typedef int                               size_type;
         typedef std::ptrdiff_t                    difference_type;
         typedef std::bidirectional_iterator_tag   iterator_category;

         explicit FaceIterator(const Block* const block, const int startingIndex = 0)
            : m_faceIndex(startingIndex),
              m_block(block)
         {
         }

         bool operator==(const FaceIterator& other) const
         {
            return m_faceIndex == other.m_faceIndex;
         }

         bool operator!=(const FaceIterator& other) const
         {
            return m_faceIndex != other.m_faceIndex;
         }

         FaceIterator operator++(int)
         {
            auto result = *this;
            ++(*this);

            return result;
         }

         FaceIterator& operator++()
         {
            if (m_faceIndex < Block::FACES_PER_BLOCK)
            {
               m_faceIndex++;
            }

            return *this;
         }

         FaceIterator operator--(int)
         {
            auto result = *this;
            --(*this);

            return result;
         }

         FaceIterator& operator--()
         {
            if (m_faceIndex > 0)
            {
               m_faceIndex--;
            }

            return *this;
         }

         const BlockFace& operator*() const
         {
            return m_block->blockFaces.at(m_faceIndex);
         }

         const BlockFace* const operator->() const
         {
            return &(m_block->blockFaces.at(m_faceIndex));
         }

      private:
         int m_faceIndex;
         const Block* const m_block;
   };
};
#endif // BLOCK_H
