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
    Regular,
    Directory,
    Symlink
};

/**
 * @brief The FileInfo struct
 */
struct FileInfo
{
    std::wstring name{};
    std::wstring extension{};

    std::uintmax_t size{ 0 };

    FileType type{ FileType::Regular };

    FileInfo() = default;

    FileInfo(std::wstring name, std::wstring extension, std::uintmax_t size, FileType type);
};

#endif // FILEINFO_H
