#ifndef BLOCK_H
#define BLOCK_H

#include "Model/precisePoint.h"

#include <QVector3D>
#include <QVector>

#include <iterator>

/**
 * @brief The Block struct represents a single file or directory in the visualization. This
 * struct constains not only the basic dimensions of the block, but also all of its vertices
 * and color information. In addition to this, there is also some metadata to aid in the creation
 * of the squarified treemap.
 */
class Block
{
    friend class SquarifiedTreeMap;

  public:
    Block() = default;

    /**
     * @brief Creates the vertices needed to represent a single block. Each face consists of
     * two triangles, and each vertex is followed by its corresponding normal. Since we are
     * unlikely to see the bottom faces of the block, no vertices (or normals) wil be dedicated to
     * visualizing it.
     *
     * @param[in] origin             The bottom-left corner of the block under construction.
     * @param[in] width              The desired block width; width grows along positive x-axis.
     * @param[in] height             The desired block height; height grows along positive y-axis.
     * @param[in] depth              The desired block depth; depth grows along negative z-axis.
     */
    Block(
        const PrecisePoint& origin, double width, double height, double depth,
        bool generateVertices = false) noexcept;

    /**
     * @brief Checks if width, height, and depth are all non-zero. It does not check to see if the
     * block is inverted (with respect to where the normals of opposing faces point).
     *
     * @returns True if the block is properly defined.
     */
    bool HasVolume() const noexcept;

    /**
     * @returns The width of the block. The width increases along the positive X axis.
     */
    double GetWidth() const noexcept;

    /**
     * @returns The height of the block. The height increases along the positive Y axis.
     */
    double GetHeight() const noexcept;

    /**
     * @returns The depth of the block. The depth increases along the negative Z axis.
     */
    double GetDepth() const noexcept;

    /**
     * @returns The origin of the block, defined as the bottom left corner of the block that is
     * closest to the origin assuming that no part of the block exists in the positive Z-space or
     * the negative X- and Y-space.
     */
    PrecisePoint GetOrigin() const noexcept;

    /**
     * @returns The current percentage of the block's surface that is covered.
     */
    double GetCoverage() const noexcept;

    /**
     * @returns All the vertices and corresponding normals that make up the block. See the
     * implementation for the exact layout.
     */
    const QVector<QVector3D>& GetVerticesAndNormals() const noexcept;

    constexpr static auto FacesPerBlock = 5;
    constexpr static auto VerticesPerBlock = 30;

  private:
    /**
     * @brief Retrieves the location at which to start the laying out immediate descendants.
     *
     * @returns The coordinates of the block's origin offset by the height of the block.
     */
    PrecisePoint ComputeNextChildOrigin() const noexcept;

    /**
     * @returns The location at which to place the next child block.
     */
    PrecisePoint GetNextRowOrigin() const noexcept;

    /**
     * @brief Stores the point at which the next child block should be placed.
     *
     * @param[in] origin          The origin at which to place the next child.
     */
    void SetNextRowOrigin(const PrecisePoint& origin) noexcept;

    /**
     * @brief Increases the percentage of the block that is covered.
     *
     * @param[in] additionalCoverage    The percentage amount by which to increase the coverage,
     *                                  expressed as a normalized val
     */
    void IncreaseCoverageBy(double additionalCoverage) noexcept;

    QVector<QVector3D> m_vertices;

    PrecisePoint m_origin;
    PrecisePoint m_nextRowOrigin; ///< Specific to the Squarified Treemap.

    double m_percentCovered = 0.0;
    double m_width = 0.0;
    double m_height = 0.0;
    double m_depth = 0.0;
};

#endif // BLOCK_H
