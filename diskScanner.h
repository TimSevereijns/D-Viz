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

#include "tree.h"

/**
 * @brief The FILE_TYPE enum
 */
enum class FILE_TYPE
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
   std::wstring m_name;
   std::uintmax_t m_size;
   FILE_TYPE m_type;

   FileInfo(const std::wstring& name, std::uintmax_t size, FILE_TYPE type)
      : m_name(name),
        m_size(size),
        m_type(type)
   {
   }
};

/**
 * @brief The Block struct
 */
struct Block
{
   QVector<QVector3D> m_vertices;
   QVector<QVector3D> m_colors;
   float m_percentCovered;
   float m_width;
   float m_height;
   float m_depth;

   Block()
      : m_width(0.0f),
        m_height(0.0f),
        m_depth(0.0f),
        m_percentCovered(0.0f)
   {
   }

   Block(const QVector3D& bottomLeft, const float width,
         const float height, const float depth)
      : m_width(width),
        m_height(height),
        m_depth(depth),
        m_percentCovered(0.0f)
   {
      const float x = bottomLeft.x();
      const float y = bottomLeft.y();
      const float z = bottomLeft.z();

      m_vertices.reserve(72);
      m_vertices
         // Front:                                               // Vertex Normals:
         << QVector3D(x           , y            , z           ) << QVector3D( 0,  0,  1)
         << QVector3D(x + width   , y            , z           ) << QVector3D( 0,  0,  1)
         << QVector3D(x           , y + height   , z           ) << QVector3D( 0,  0,  1)
         << QVector3D(x + width   , y + height   , z           ) << QVector3D( 0,  0,  1)
         << QVector3D(x           , y + height   , z           ) << QVector3D( 0,  0,  1)
         << QVector3D(x + width   , y            , z           ) << QVector3D( 0,  0,  1)
         // Right:
         << QVector3D(x + width   , y            , z           ) << QVector3D( 1,  0,  0)
         << QVector3D(x + width   , y            , z - depth   ) << QVector3D( 1,  0,  0)
         << QVector3D(x + width   , y + height   , z           ) << QVector3D( 1,  0,  0)
         << QVector3D(x + width   , y + height   , z - depth   ) << QVector3D( 1,  0,  0)
         << QVector3D(x + width   , y + height   , z           ) << QVector3D( 1,  0,  0)
         << QVector3D(x + width   , y            , z - depth   ) << QVector3D( 1,  0,  0)
         // Back:
         << QVector3D(x + width   , y            , z - depth   ) << QVector3D( 0,  0, -1)
         << QVector3D(x           , y            , z - depth   ) << QVector3D( 0,  0, -1)
         << QVector3D(x + width   , y + height   , z - depth   ) << QVector3D( 0,  0, -1)
         << QVector3D(x           , y + height   , z - depth   ) << QVector3D( 0,  0, -1)
         << QVector3D(x + width   , y + height   , z - depth   ) << QVector3D( 0,  0, -1)
         << QVector3D(x           , y            , z - depth   ) << QVector3D( 0,  0, -1)
         // Left:
         << QVector3D(x           , y            , z - depth   ) << QVector3D(-1,  0,  0)
         << QVector3D(x           , y            , z           ) << QVector3D(-1,  0,  0)
         << QVector3D(x           , y + height   , z - depth   ) << QVector3D(-1,  0,  0)
         << QVector3D(x           , y + height   , z           ) << QVector3D(-1,  0,  0)
         << QVector3D(x           , y + height   , z - depth   ) << QVector3D(-1,  0,  0)
         << QVector3D(x           , y            , z           ) << QVector3D(-1,  0,  0)
         // Bottom:
         << QVector3D(x           , y            , z - depth   ) << QVector3D( 0, -1,  0)
         << QVector3D(x + width   , y            , z - depth   ) << QVector3D( 0, -1,  0)
         << QVector3D(x           , y            , z           ) << QVector3D( 0, -1,  0)
         << QVector3D(x + width   , y            , z           ) << QVector3D( 0, -1,  0)
         << QVector3D(x           , y            , z           ) << QVector3D( 0, -1,  0)
         << QVector3D(x + width   , y            , z - depth   ) << QVector3D( 0, -1,  0)
         // Top:
         << QVector3D(x           , y + height   , z           ) << QVector3D( 0,  1,  0)
         << QVector3D(x + width   , y + height   , z           ) << QVector3D( 0,  1,  0)
         << QVector3D(x           , y + height   , z - depth   ) << QVector3D( 0,  1,  0)
         << QVector3D(x + width   , y + height   , z - depth   ) << QVector3D( 0,  1,  0)
         << QVector3D(x           , y + height   , z - depth   ) << QVector3D( 0,  1,  0)
         << QVector3D(x + width   , y + height   , z           ) << QVector3D( 0,  1,  0);
   }

   bool IsDefined()
   {
      return (m_width > 0.0f && m_height > 0.0f && m_depth > 0.0f);
   }
};


/**
 * @brief The VizNode struct
 */
struct VizNode
{
   FileInfo m_file;
   Block m_block;

   VizNode(const FileInfo& file)
      : m_file(file),
        m_block()
   {
   }

   VizNode(const FileInfo& file, const Block& block)
      : m_file(file),
        m_block(block)
   {
   }
};

class QJsonObject;

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
       * @brief ToJSON serializes the scanned tree into a QJsonObject.
       * @param[out] json           The QJsonObject to parse the file tree into.
       */
      void ToJSON(QJsonObject& json);

      /**
       * @brief Scan will perform a recursive scan of the filesystem, starting at the specified
       * path.
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
      Tree<VizNode>& GetDirectoryTree() const;

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

      std::unique_ptr<Tree<VizNode>> m_fileTree;

      boost::filesystem::path m_path;

      std::thread m_scanningThread;
      std::mutex m_mutex;

      std::uintmax_t m_filesScanned;

      std::chrono::duration<double> m_scanningTime;
};

#endif // DISKSCANNER_H
