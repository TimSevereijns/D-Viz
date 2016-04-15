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
   std::wstring name{ };
   std::uintmax_t size{ 0 };
   FILE_TYPE type{ FILE_TYPE::REGULAR };

   FileInfo() = default;

   FileInfo(const std::wstring& name, std::uintmax_t size, FILE_TYPE type);
};

#endif // FILEINFO_H
