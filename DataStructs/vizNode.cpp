#include "vizNode.h"

std::uint32_t VizNode::INVALID_OFFSET = std::numeric_limits<std::uint32_t>::max();

VizNode::VizNode(const FileInfo& file)
   : file(file),
     block(),
     boundingBox(),
     offsetIntoVBO(VizNode::INVALID_OFFSET)
{
}

VizNode::VizNode(const FileInfo& file, const Block& block)
   : file(file),
     block(block),
     boundingBox(block),
     offsetIntoVBO(VizNode::INVALID_OFFSET)
{
}
