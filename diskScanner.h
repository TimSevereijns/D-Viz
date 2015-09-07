#ifndef DISKSCANNER_H
#define DISKSCANNER_H

#include <boost/filesystem.hpp>

#include <QVector>
#include <QVector3D>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "DataStructs/block.h"
#include "DataStructs/fileInfo.h"
#include "DataStructs/vizNode.h"

#include "tree.h"

/**
 * @brief The DiskScanner class
 */
class DiskScanner
{
   public:
      static const std::uintmax_t SIZE_UNDEFINED;

      explicit DiskScanner();
      explicit DiskScanner(const std::wstring& rawPpath);

      void PrintTree() const;
      void PrintTreeMetadata() const;

      /**
       * @brief ToJSON serializes the scanned tree into a QJsonObject.
       * @param[out] json           The QJsonObject to parse the file tree into.
       */
      void ToJSON(QJsonObject& json);

      /**
       * @brief Scan will perform a recursive scan of the filesystem, starting at the specified
       * path, and it will then compute and update the size of any directories in the tree.
       * @param[out] progress       An atomic containing the number of files scanned so far, and a
       *                            boolean that will be set to true once scanning has completed.
       */
      void StartScanning(std::atomic<std::pair<std::uintmax_t, bool>>* progress);

      /**
       * @brief ScanInNewThread kicks off the filesystem scan in a new thread.
       * @param[out] progress       An atomic containing the number of files scanned so far, and a
       *                            boolean that will be set to true once scanning has completed.
       */
      void ScanInNewThread(std::atomic<std::pair<std::uintmax_t, bool>>* progress);

      /**
       * @brief JoinScanningThread wraps the call to std::thread::join.
       */
      void JoinScanningThread();

      /**
       * @brief ComputeDirectorySizes traverses the file-tree and computes the size of each
       * directory.
       */
      void ComputeDirectorySizes();

      /**
       * @brief GetNumberOfFilesScanned reports filesystem scanning progress.
       * @returns the number of files that have been scanned so far.
       */
      std::uintmax_t GetNumberOfFilesScanned();

      /**
       * @brief GetDirectoryTree
       * @return
       */
      std::shared_ptr<Tree<VizNode>> GetFileTree() const;

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
       * Max path length in Windows is 260 characters, so if that includes slashes, then the maximum
       * depth of a directory or file is no more than 130, or so. Given that the default stack size
       * in MSVC is 1MB, and I only pass in references, this recursive version may be fine---maybe!
       */
      void ScanRecursively(const boost::filesystem::path& path, TreeNode<VizNode>& fileNode,
         std::atomic<std::pair<std::uintmax_t, bool>>* progress);

      std::shared_ptr<Tree<VizNode>> m_fileTree;

      boost::filesystem::path m_path;

      std::thread m_scanningThread;
      std::mutex m_mutex;

      std::uintmax_t m_filesScanned;

      std::chrono::duration<double> m_scanningTime;
};

#endif // DISKSCANNER_H
