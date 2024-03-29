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
class VizBlock;

namespace Scanner
{
    namespace Detail
    {
#ifdef Q_OS_WIN
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
    }  // namespace Detail

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
     * @returns True if the given path represents a reparse point, and false otherwise.
     */
    bool IsReparsePoint(const std::filesystem::path& path) noexcept;
#endif // Q_OS_WIN

} // namespace Scanner

#endif // DRIVESCANNINGUTILITIES_H
