#include "fileInfo.h"

FileInfo::FileInfo(std::wstring name, std::wstring extension, std::uintmax_t size, FileType type)
    : name{ std::move(name) }, extension{ std::move(extension) }, size{ size }, type{ type }
{
}
