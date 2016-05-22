#include "fileInfo.h"

FileInfo::FileInfo(
   const std::wstring& name,
   std::uintmax_t size,
   FileType type)
   :
   name{ name },
   size{ size },
   type{ type }
{
}
