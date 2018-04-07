#include "vizBlock.h"

VizBlock::VizBlock(const FileInfo& file) :
   file{ file }
{
}

VizBlock::VizBlock(
   const FileInfo& file,
   const Block& block)
   :
   file{ file },
   block{ block },
   boundingBox{ block }
{
}
