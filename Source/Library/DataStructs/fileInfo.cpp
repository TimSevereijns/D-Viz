#include "fileInfo.h"

FileInfo::FileInfo(
   const std::wstring& name,
   const std::wstring& extension,
   std::uintmax_t size,
   FileType type)
   :
   name{ name },
   extension{ extension },
   size{ size },
   type{ type }
{
}
