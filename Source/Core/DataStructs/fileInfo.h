#ifndef FILEINFO_H
#define FILEINFO_H

#include <cstdint>
#include <string>

/**
 * @brief The FILE_TYPE enum represents the three basic file types: non-directory files,
 * directories, and symbolic links (which includes junctions).
 */
enum class FileType
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
   std::wstring extension{ };

   std::uintmax_t size{ 0 };

   FileType type{ FileType::REGULAR };

   FileInfo() = default;

   FileInfo(
      const std::wstring& name,
      const std::wstring& extension,
      std::uintmax_t size,
      FileType type);
};

#endif // FILEINFO_H
