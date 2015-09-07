#ifndef VIZNODE_H
#define VIZNODE_H

#include "fileInfo.h"
#include "block.h"

/**
 * @brief The VizNode struct
 */
struct VizNode
{
   FileInfo m_file;
   Block m_block;

   explicit VizNode(const FileInfo& file);

   VizNode(const FileInfo& file, const Block& block);
};


#endif // VIZNODE_H
