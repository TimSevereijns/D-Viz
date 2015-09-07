#include "fileInfo.h"

FileInfo::FileInfo(const std::wstring& name, std::uintmax_t size, FILE_TYPE type)
   : m_name(name),
     m_size(size),
     m_type(type)
{
}
