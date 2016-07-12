#ifndef BLOCK_H
#define BLOCK_H

#include "doublePoint3d.h"

#include <QVector>
#include <QVector3D>

#include <iterator>

/**
 * @brief The Block struct represents a single file or directory in the visualization. This
 * struct constains not only the basic dimensions of the block, but also all of its vertices
 * and color information. In addition to this, there is also some metadata to aid in the creation
 * of the squarified treemap.
 */
class Block
{
   public:

      Block() = default;

      /**
       * @brief Block creates the vertices needed to represent a single block. Each face consists of
       * two triangles, and each vertex is followed by its corresponding normal. Since we are
       * unlikely to see the bottom faces of the block, no vertices (or normals) wil be dedicated to
       * visualizing it.
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
         const double depth,
         const bool generateVertices = false);

      /**
       * @brief IsDefined checks if width, height, and depth are all non-zero. It does not check
       * to see if the block is inverted (with respect to where the normals of opposing faces
       * point).
       *
       * @returns True if the block is properly defined.
       */
      bool HasVolume() const;

      /**
       * @brief GetNextChildOrigin will return the location at which to start the laying out
       * immediate descendants.
       *
       * @returns The coordinates of the block's origin offset by the height of the block.
       */
      DoublePoint3D ComputeNextChildOrigin() const;

      /**
       * @brief GetWidth
       * @return
       */
      double GetWidth() const;

      /**
       * @brief GetWidth
       * @return
       */
      double GetHeight() const;

      /**
       * @brief GetWidth
       * @return
       */
      double GetDepth() const;

      /**
       * @brief GetOrigin
       * @return
       */
      DoublePoint3D GetOrigin() const;

      /**
       * @brief GetNextRowOrigin
       * @return
       */
      DoublePoint3D GetNextRowOrigin() const;

      /**
       * @brief SetNextRowOrigin
       * @param origin
       */
      void SetNextRowOrigin(const DoublePoint3D& origin);

      /**
       * @brief GetCoverage
       * @return
       */
      double GetCoverage() const;

      /**
       * @brief IncreaseCoverageBy
       * @param additionalCoverage
       */
      void IncreaseCoverageBy(double additionalCoverage);

      /**
       * @brief GetVertices
       * @return
       */
      const QVector<QVector3D>& GetVertices() const;

      constexpr static auto FACES_PER_BLOCK{ 5 };
      constexpr static auto VERTICES_PER_BLOCK{ 30 };

   private:
      QVector<QVector3D> m_vertices;

      DoublePoint3D m_origin;
      DoublePoint3D m_nextRowOrigin; ///< Specific to the Squarified Treemap.

      double m_percentCovered{ 0.0 };
      double m_width{ 0.0 };
      double m_height{ 0.0 };
      double m_depth{ 0.0 };
};
#endif // BLOCK_H
