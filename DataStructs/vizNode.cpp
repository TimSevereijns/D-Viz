#include "vizNode.h"

VizNode::VizNode(const FileInfo& file)
   : m_file(file),
     m_block()
{
}

VizNode::VizNode(const FileInfo& file, const Block& block)
   : m_file(file),
     m_block(block)
{
}
