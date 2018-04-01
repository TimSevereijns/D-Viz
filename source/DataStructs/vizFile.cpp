#include "vizFile.h"

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
