#ifndef VIZNODE_H
#define VIZNODE_H

#include <limits>

#include "Model/Scanner/fileInfo.h"
#include "Model/block.h"

/**
 * @brief Represents everything needed to parse, render, and perform hit detection on an individual
 * file as identified during the scanning process.
 */
class VizBlock
{
  public:
    VizBlock() = default;

    /**
     * @brief Constructs a new VizNode to represent the specified file.
     *
     * The block, and its bounding box will be default constructed.
     *
     * @param[in] file               The file that the VizNode represents.
     */
    explicit VizBlock(FileInfo file);

    constexpr static auto NotInVBO = std::numeric_limits<std::uint32_t>::max();

    FileInfo file{};     ///< The file that the block represents.
    Block block{};       ///< The actual block as rendered to the OpenGL canvas.
    Block boundingBox{}; ///< Minimum axis-aligned bounding box for node and all descendents.

    /** The offset of this node into the VBO once the visualization has been generated */
    std::uint32_t offsetIntoVBO{ VizBlock::NotInVBO };
};

#endif // VIZNODE_H
