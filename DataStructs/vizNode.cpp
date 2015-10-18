#include "vizNode.h"

VizNode::VizNode(const FileInfo& file)
   : file(file),
     block()
{
}

VizNode::VizNode(const FileInfo& file, const Block& block)
   : file(file),
     block(block)
{
}
