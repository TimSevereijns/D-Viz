#ifndef DISKSCANNER_H
#define DISKSCANNER_H

#include <boost/filesystem.hpp>

#include <cstdint>
#include <memory>
#include <string>

#include "tree.h"

class DiskScanner
{
   public:
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
      std::unique_ptr<Tree<boost::filesystem::path>> m_fileTree;
};

#endif // DISKSCANNER_H
