#ifndef DRIVESCANNINGUTILITIES_H
#define DRIVESCANNINGUTILITIES_H

#include <QtGlobal>

#ifdef Q_OS_WIN
#include <Windows.h>
#include <minwindef.h>

#include "Utilities/scopedHandle.h"
#endif // Q_OS_WIN

#include <filesystem>

template <typename T> class Tree;

struct VizBlock;

namespace Scanner
{
    namespace Detail
    {

#ifdef Q_OS_WIN

        /**
         * @returns A handle representing the repartse point found at the given path. If the path is
         * not a reparse point, then an invalid handle will be returned instead.
         */
        ScopedHandle OpenReparsePoint(const std::filesystem::path& path) noexcept;

        /**
         * @brief Reads the reparse point found at the given path into the output buffer.
         *
         * @returns True if the path could be read as a reparse point, and false otherwise.
         */
        bool
        ReadReparsePoint(const std::wstring& path, std::vector<std::byte>& reparseBuffer) noexcept;

        /**
         * @brief Use the `FindFirstFileW(...)` function to retrieve the file size.
         *
         * The `std::filesystem::file_size(...)` function uses a different native
         * function to get at the file size for a given file, and this function (while probably
         * faster than `FindFirstFileW(...)`) has a tendency to throw. If such exceptional behaviour
         * were to occur, then this function can be used to hopefully still get at the file size.
         *
         * @param path[in]               The path to the troublesome file.
         *
         * @returns The size of the file if it's accessible, and zero otherwise.
         */
        std::uintmax_t GetFileSizeUsingWinAPI(const std::filesystem::path& path) noexcept;

#endif // Q_OS_WIN

    } // namespace Detail

    /**
     * @brief Helper function to safely wrap the computation of a file's size.
     *
     * @param path[in]               The path to the file.
     *
     * @return The size of the file if it's accessible, and zero otherwise.
     */
    std::uintmax_t ComputeFileSize(const std::filesystem::path& path) noexcept;

    /**
     * @brief ComputeDirectorySizes
     *
     * @param tree
     */
    void ComputeDirectorySizes(Tree<VizBlock>& tree) noexcept;

#ifdef Q_OS_WIN

    /**
     * @returns True if the given file path matches the given reparse tag, and false otherwise.
     */
    bool IsReparseTag(const std::filesystem::path& path, DWORD targetTag) noexcept;

    /**
     * @note Junctions in Windows are considered mount points.
     *
     * @returns True if the given file path represents a mount point, and false otherwise.
     */
    bool IsMountPoint(const std::filesystem::path& path) noexcept;

    /**
     * @returns True if the given file path represents a symlink, and false otherwise.
     */
    bool IsSymlink(const std::filesystem::path& path) noexcept;

    /**
     * @returns True if the given path represents a reparse point, and false otherwise.
     */
    bool IsReparsePoint(const std::filesystem::path& path) noexcept;

#endif // Q_OS_WIN

} // namespace Scanner

#endif // DRIVESCANNINGUTILITIES_H
