#ifndef DISKSCANNER_H
#define DISKSCANNER_H

#include <boost/filesystem.hpp>

#include <cstdint>
#include <memory>
#include <string>

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
      std::unique_ptr<Tree<FileInfo>> m_fileTree;
};

#endif // DISKSCANNER_H
