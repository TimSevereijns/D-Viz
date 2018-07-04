#ifndef DRIVESCANNINGUTILITIES_H
#define DRIVESCANNINGUTILITIES_H

#include <QtGlobal>

#ifdef Q_OS_WIN
   #include <Windows.h>
   #include <minwindef.h>

   #include "scopedHandle.h"
#endif // Q_OS_WIN

#include <experimental/filesystem>

template<typename T>
class Tree;

struct VizBlock;

namespace DriveScanning
{
   namespace Detail
   {
#ifdef Q_OS_WIN

      /**
      * @returns A handle representing the repartse point found at the given path. If
      * the path is not a reparse point, then an invalid handle will be returned instead.
      */
      auto OpenReparsePoint(
         const std::experimental::filesystem::path& path) noexcept -> ScopedHandle;

      /**
      * @brief Reads the reparse point found at the given path into the output buffer.
      *
      * @returns True if the path could be read as a reparse point, and false otherwise.
      */
      auto ReadReparsePoint(
         const std::wstring& path,
         std::vector<std::byte>& reparseBuffer) noexcept -> bool;

      /**
       * @brief Use the `FindFirstFileW(...)` function to retrieve the file size.
       *
       * The `std::experimental::filesystem::file_size(...)` function uses a different native
       * function to get at the file size for a given file, and this function (while probably faster
       * than `FindFirstFileW(...)`) has a tendency to throw. If such exceptional behaviour were to
       * occur, then this function can be used to hopefully still get at the file size.
       *
       * @param path[in]               The path to the troublesome file.
       *
       * @returns The size of the file if it's accessible, and zero otherwise.
       */
      auto GetFileSizeUsingWinAPI(
         const std::experimental::filesystem::path& path) noexcept -> std::uintmax_t;

#endif // Q_OS_WIN
   }

   namespace Utilities
   {
      /**
       * @brief Helper function to safely wrap the computation of a file's size.
       *
       * @param path[in]               The path to the file.
       *
       * @return The size of the file if it's accessible, and zero otherwise.
       */
      auto ComputeFileSize(
         const std::experimental::filesystem::path& path) noexcept -> std::uintmax_t;

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
      auto IsReparseTag(
         const std::experimental::filesystem::path& path,
         DWORD targetTag) noexcept -> bool;

      /**
      * @note Junctions in Windows are considered mount points.
      *
      * @returns True if the given file path represents a mount point, and false otherwise.
      */
      auto IsMountPoint(const std::experimental::filesystem::path& path) noexcept -> bool;

      /**
      * @returns True if the given file path represents a symlink, and false otherwise.
      */
      auto IsSymlink(const std::experimental::filesystem::path& path) noexcept -> bool;

      /**
      * @returns True if the given path represents a reparse point, and false otherwise.
      */
      auto IsReparsePoint(const std::experimental::filesystem::path& path) noexcept -> bool;

#endif // Q_OS_WIN
   }
}

#endif // DRIVESCANNINGUTILITIES_H
