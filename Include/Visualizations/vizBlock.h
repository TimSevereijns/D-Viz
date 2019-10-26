#ifndef VIZNODE_H
#define VIZNODE_H

#include <limits>

#include "Scanner/fileInfo.h"
#include "Visualizations/block.h"

/**
 * @brief The VizNode struct represents everything needed to parse, render, and perform hit
 * detection on an individual file as identified during the scanning process.
 */
struct VizBlock
{
    constexpr static auto INVALID_OFFSET = std::numeric_limits<std::uint32_t>::max();

    FileInfo file{};     ///< The file that the block represents.
    Block block{};       ///< The actual block as rendered to the OpenGL canvas.
    Block boundingBox{}; ///< Minimum axis-aligned bounding box for node and all descendents.

    /** The offset of this node into the VBO once the visualization has been generated */
    std::uint32_t offsetIntoVBO{ VizBlock::INVALID_OFFSET };

    VizBlock() = default;

    /**
     * @brief Constructs a new VizNode to represent the specified file.
     *
     * The block, and its bounding box will be default constructed.
     *
     * @param[in] file               The file that the VizNode represents.
     */
    explicit VizBlock(FileInfo file);

    /**
     * @brief Constructs a new VizNode to represent the specified file using the specified
     * block.
     *
     * The bounding box will be initialized to the same size as the block.
     *
     * @param[in] file               The file that the VizNode represents.
     * @param[in] block              The visual representation of the file in question.
     */
    VizBlock(FileInfo file, const Block& block);
};

#endif // VIZNODE_H
