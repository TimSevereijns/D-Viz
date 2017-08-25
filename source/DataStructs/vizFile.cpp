#include "vizFile.h"

std::uint32_t VizFile::INVALID_OFFSET = std::numeric_limits<std::uint32_t>::max();

VizFile::VizFile(const FileInfo& file) :
   file{ file }
{
}

VizFile::VizFile(
   const FileInfo& file,
   const Block& block)
   :
   file{ file },
   block{ block },
   boundingBox{ block }
{
}
