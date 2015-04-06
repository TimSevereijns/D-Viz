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

   QVector3D m_nextRowOrigin; // Specific to the Squarified Treemap.

   double m_percentCovered;
   double m_width;
   double m_height;
   double m_depth;

   Block()
      : m_width(0.0),
        m_height(0.0),
        m_depth(0.0),
        m_percentCovered(0.0)
   {
   }

   /**
    * @brief Block creates the vertices needed to represent a single block. Each
    *        face consists of two triangles, and each vertex is followed by its corresponding
    *        normal. Since we are unlikely to see the bottom faces of the block, no vertices (or
    *        normals) wil be dedicated to visualizing it.
    *
    * @param bottomLeft             The bottom-left corner of the block under construction.
    * @param width                  The desired block width; width grows along positive x-axis.
    * @param height                 The desired block height; height grows along positive y-axis.
    * @param depth                  The desired block depth; depth grows along negative z-axis.
    *
    * @returns a vector of vertices.
    */
   Block(const QVector3D& bottomLeft, const double width, const double height, const double depth)
      : m_width(width),
        m_height(height),
        m_depth(depth),
        m_percentCovered(0.0),
        m_nextRowOrigin(bottomLeft.x(), bottomLeft.y() + height, bottomLeft.z())
   {
      const float x = bottomLeft.x();
      const float y = bottomLeft.y();
      const float z = bottomLeft.z();

      m_vertices.reserve(60);
      m_vertices
         // Front:                                               // Vertex Normals:        // Index:
         << QVector3D(x           , y            , z           ) << QVector3D( 0,  0,  1)  // 0
         << QVector3D(x + width   , y            , z           ) << QVector3D( 0,  0,  1)  // 2
         << QVector3D(x           , y + height   , z           ) << QVector3D( 0,  0,  1)  // 4
         << QVector3D(x + width   , y + height   , z           ) << QVector3D( 0,  0,  1)  // 6
         << QVector3D(x           , y + height   , z           ) << QVector3D( 0,  0,  1)  // 8
         << QVector3D(x + width   , y            , z           ) << QVector3D( 0,  0,  1)  // 10
         // Right:
         << QVector3D(x + width   , y            , z           ) << QVector3D( 1,  0,  0)  // 12
         << QVector3D(x + width   , y            , z - depth   ) << QVector3D( 1,  0,  0)  // 14
         << QVector3D(x + width   , y + height   , z           ) << QVector3D( 1,  0,  0)  // 16
         << QVector3D(x + width   , y + height   , z - depth   ) << QVector3D( 1,  0,  0)  // 18
         << QVector3D(x + width   , y + height   , z           ) << QVector3D( 1,  0,  0)  // 20
         << QVector3D(x + width   , y            , z - depth   ) << QVector3D( 1,  0,  0)  // 22
         // Back:
         << QVector3D(x + width   , y            , z - depth   ) << QVector3D( 0,  0, -1)  // 24
         << QVector3D(x           , y            , z - depth   ) << QVector3D( 0,  0, -1)  // 26
         << QVector3D(x + width   , y + height   , z - depth   ) << QVector3D( 0,  0, -1)  // 28
         << QVector3D(x           , y + height   , z - depth   ) << QVector3D( 0,  0, -1)  // 30
         << QVector3D(x + width   , y + height   , z - depth   ) << QVector3D( 0,  0, -1)  // 32
         << QVector3D(x           , y            , z - depth   ) << QVector3D( 0,  0, -1)  // 34
         // Left:
         << QVector3D(x           , y            , z - depth   ) << QVector3D(-1,  0,  0)  // 36
         << QVector3D(x           , y            , z           ) << QVector3D(-1,  0,  0)  // 38
         << QVector3D(x           , y + height   , z - depth   ) << QVector3D(-1,  0,  0)  // 40
         << QVector3D(x           , y + height   , z           ) << QVector3D(-1,  0,  0)  // 42
         << QVector3D(x           , y + height   , z - depth   ) << QVector3D(-1,  0,  0)  // 44
         << QVector3D(x           , y            , z           ) << QVector3D(-1,  0,  0)  // 46
         // Top:
         << QVector3D(x           , y + height   , z           ) << QVector3D( 0,  1,  0)  // 48
         << QVector3D(x + width   , y + height   , z           ) << QVector3D( 0,  1,  0)  // 50
         << QVector3D(x           , y + height   , z - depth   ) << QVector3D( 0,  1,  0)  // 52
         << QVector3D(x + width   , y + height   , z - depth   ) << QVector3D( 0,  1,  0)  // 54
         << QVector3D(x           , y + height   , z - depth   ) << QVector3D( 0,  1,  0)  // 56
         << QVector3D(x + width   , y + height   , z           ) << QVector3D( 0,  1,  0); // 58
   }

   /**
    * @brief IsDefined checks if width, height, and depth are all non-zero. It does not check
    * to see if the block is inverted (with respect to where the normals of opposing faces point);
    * call IsValid() to perform that check.
    *
    * @returns true if the block is properly defined.
    */
   bool IsDefined() const
   {
      return (m_width != 0.0 && m_height != 0.0 && m_depth != 0.0);
   }

   /**
    * @brief IsValid performs a quick check of Cartesian X-axis coordinates to determine if the
    * block is in a valid state.
    *
    * @returns true if the block is defined and the left face is indeed to the left of the right
    * face; false otherwise.
    */
   bool IsValid() const
   {
      if (IsDefined())
      {
         return false;
      }

      // The indices used are keyed off of the vertex order used in the constructor above.
      return m_vertices[36].x() < m_vertices[12].x();
   }

   /**
    * @brief GetOriginPlusHeight
    * @returns the coordinates of the block's origin offset by the height of the block.
    */
   QVector3D GetOriginPlusHeight() const
   {
      QVector3D origin = m_vertices.front();
      origin += QVector3D(0, m_height, 0);

      return origin;
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
