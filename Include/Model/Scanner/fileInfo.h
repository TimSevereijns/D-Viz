#ifndef FILEINFO_H
#define FILEINFO_H

#include <cstdint>
#include <string>

/**
 * @brief Represents the three basic file types: non-directory files,
 * directories, and symbolic links (which includes reparse points on Windows).
 */
enum class FileType
{
    Regular,
    Directory,
    Symlink
};

/**
 * @brief A wrapper around various pieces of file metadata.
 */
struct FileInfo
{
    FileInfo() = default;

    FileInfo(std::wstring name, std::wstring extension, std::uintmax_t size, FileType type) noexcept
        : name{ std::move(name) }, extension{ std::move(extension) }, size{ size }, type{ type }
    {
    }

    std::wstring name;
    std::wstring extension;

    std::uint32_t identifier{ 0 };
    std::uintmax_t size{ 0 };

    FileType type{ FileType::Regular };
};

#endif // FILEINFO_H
