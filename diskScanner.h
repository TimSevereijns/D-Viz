#ifndef DISKSCANNER_H
#define DISKSCANNER_H

#include <boost/filesystem.hpp>

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "tree.h"

enum class FILE_TYPE
{
   REGULAR,
   DIRECTORY,
   SYMLINK
};

struct FileInfo
{
   std::wstring m_name;
   std::uintmax_t m_size;
   FILE_TYPE m_type;

   FileInfo(std::wstring name, std::uintmax_t size, FILE_TYPE type)
      : m_name(name),
        m_size(size),
        m_type(type)
   {
   }
};

class DiskScanner
{
   public:
      static std::uintmax_t SIZE_UNDEFINED;

      explicit DiskScanner();
      explicit DiskScanner(const std::wstring& rawPpath);
      ~DiskScanner();

      void PrintTree() const;
      void PrintTreeMetadata() const;

      void Scan(std::atomic<std::pair<std::uintmax_t, bool>>* progress);

      void ScanInNewThread(std::atomic<std::pair<std::uintmax_t, bool>>* progress);

      void JoinScanningThread();

      std::uintmax_t GetNumberOfFilesScanned();

      /**
       * @brief ConvertBytesToMegaBytes Converts a size in bytes to megabytes.
       * @param bytes               The value in bytes to be converted.
       * @returns The converted result.
       */
      static double ConvertBytesToMegaBytes(const std::uintmax_t bytes);

      /**
       * @brief ConvertBytesToGigaBytes Converts a size in bytes to gigabytes.
       * @param bytes               The value in bytes to be converted.
       * @returns The converted result.
       */
      static double ConvertBytesToGigaBytes(const std::uintmax_t bytes);

   private:
      /**
       * TODO: Consider replacing this recursive version with an iterative implementation.
       *
       * Max path length in Windows is 260 characters, so if that includes slashes, then the maximum
       * depth of a directory or file is no more than 130, or so. Given that the default stack size
       * in MSVC is 1MB, and I only pass in references, this recursive version may be fine---maybe!
       */
      template<typename T>
      void ScanRecursively(const boost::filesystem::path& path, TreeNode<T>& fileNode,
         std::atomic<std::pair<std::uintmax_t, bool>>* progress);

      std::unique_ptr<Tree<FileInfo>> m_fileTree;

      boost::filesystem::path m_path;

      std::thread m_scanningThread;
      std::mutex m_mutex;

      std::uintmax_t m_filesScanned;
};

#endif // DISKSCANNER_H
