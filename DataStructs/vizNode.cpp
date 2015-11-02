#include "vizNode.h"

VizNode::VizNode(const FileInfo& file)
   : file(file),
     block(),
     boundingBox()
{
}

VizNode::VizNode(const FileInfo& file, const Block& block)
   : file(file),
     block(block)
{
}
