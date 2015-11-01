#ifndef VIZNODE_H
#define VIZNODE_H

#include "fileInfo.h"
#include "block.h"

/**
 * @brief The VizNode struct represents everything needed to parse, render, and perform hit
 * detection on an individual file as identified during the scanning process.
 */
struct VizNode
{
   FileInfo file;       ///< The file that the block represents.
   Block block;         ///< The actual block as rendered to the OpenGL canvas.
   Block boundingBox;   ///< Minimum axis-aligned bounding box for node and all descendents.

   explicit VizNode(const FileInfo& file);

   VizNode(const FileInfo& file, const Block& block);
};

#endif // VIZNODE_H
