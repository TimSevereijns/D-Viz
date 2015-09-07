#ifndef FILEINFO_H
#define FILEINFO_H

#include <cstdint>
#include <string>

/**
 * @brief The FILE_TYPE enum represents the three basic file types: non-directory files,
 * directories, and symbolic links (which includes junctions).
 */
enum class FILE_TYPE
{
   REGULAR,
   DIRECTORY,
   SYMLINK
};

/**
 * @brief The FileInfo struct
 */
struct FileInfo
{
   std::wstring m_name;
   std::uintmax_t m_size;
   FILE_TYPE m_type;

   FileInfo(const std::wstring& name, std::uintmax_t size, FILE_TYPE type);
};

#endif // FILEINFO_H
