#ifndef VIZNODE_H
#define VIZNODE_H

#include <limits>

#include "fileInfo.h"
#include "block.h"

/**
 * @brief The VizNode struct represents everything needed to parse, render, and perform hit
 * detection on an individual file as identified during the scanning process.
 */
struct VizNode
{
   static std::uint32_t INVALID_OFFSET;

   FileInfo file{ };          ///< The file that the block represents.
   Block block{ };            ///< The actual block as rendered to the OpenGL canvas.
   Block boundingBox{ };      ///< Minimum axis-aligned bounding box for node and all descendents.

   /** The offset of this node into the VBO once the visualization has been generated */
   std::uint32_t offsetIntoVBO{ VizNode::INVALID_OFFSET };

   /**
    * @brief VizNode constructs a new VizNode to represent the specified file.
    *
    * The block, and its bounding box will be default constructed.
    *
    * @param[in] file               The file that the VizNode represents.
    */
   explicit VizNode(const FileInfo& file);

   /**
    * @brief VizNode constructs a new VizNode to represent the specified file using the specified
    * block.
    *
    * The bounding box will be initialized to the same size as the block.
    *
    * @param[in] file               The file that the VizNode represents.
    * @param[in] block              The visual representation of the file in question.
    */
   explicit VizNode(
      const FileInfo& file,
      const Block& block);
};

#endif // VIZNODE_H
