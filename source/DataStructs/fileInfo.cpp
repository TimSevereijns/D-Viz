#include "fileInfo.h"

FileInfo::FileInfo(
   const std::wstring& name,
   std::uintmax_t size,
   FILE_TYPE type)
   :
   name{ name },
   size{ size },
   type{ type }
{
}
