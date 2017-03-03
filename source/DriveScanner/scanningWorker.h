#ifndef SCANNINGWORKER_H
#define SCANNINGWORKER_H

#include <experimental/filesystem>

#include <QObject>
#include <QVector>
#include <QVector3D>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

#include "../DataStructs/block.h"
#include "../DataStructs/driveScanningParameters.h"
#include "../DataStructs/fileInfo.h"
#include "../DataStructs/scanningprogress.hpp"
#include "../DataStructs/vizNode.h"

#include "../Visualizations/visualization.h"

#include "../ThirdParty/Tree.hpp"

/**
 * @brief The NodeAndPath struct
 */
struct NodeAndPath
{
   std::unique_ptr<TreeNode<VizNode>> node;
   std::experimental::filesystem::path path;

   NodeAndPath(
      decltype(node) node,
      decltype(path) path)
      :
      node{ std::move(node) },
      path{ std::move(path) }
   {
   }

   NodeAndPath() = default;
};

template<typename Type>
class ThreadSafeQueue;

/**
 * @brief The ScanningWorker class
 */
class ScanningWorker : public QObject
{
   Q_OBJECT

   public:

      static constexpr std::uintmax_t SIZE_UNDEFINED{ 0 };

      explicit ScanningWorker(
         const DriveScanningParameters& parameters,
         ScanningProgress& progress);

   public slots:

      /**
       * @brief Kicks off the drive scanning process.
       *
       * As part of the scanning process, the ProgressUpdate signal will be fired to signal progress
       * updates, and the Finish signal will be fired once the scanning process completes
       * successfully.
       *
       * @see Finished
       * @see ProgressUpdate
       */
      void Start();

   signals:

      /**
       * @brief Signals that the drive scanning has finished.
       *
       * @param[in] fileTree        A pointer to the final tree representing the scanned drive.
       */
      void Finished(std::shared_ptr<Tree<VizNode>> fileTree);

      /**
       * @brief Signals drive scanning progress updates.
       */
      void ProgressUpdate();

      /**
       * @brief Allows for cross-thread signaling to show the user a standard Qt message box.
       *
       * @param[in] message         The message to be shown in the message box.
       */
      void ShowMessageBox(const QString& message);

   private:

      void ProcessQueue(
         ThreadSafeQueue<NodeAndPath>& taskQueue,
         ThreadSafeQueue<NodeAndPath>& resultsQueue) noexcept;

      /**
       * @brief Helper function to process a single file.
       *
       * @note This function assumes the path is valid and accessible.
       *
       * @param[in] path            The location on disk to scan.
       * @param[in] fileNode        The TreeNode in Tree to append newly discoved files to.
       */
      void ProcessFile(
         const std::experimental::filesystem::path& path,
         TreeNode<VizNode>& treeNode) noexcept;

      /**
       * @brief Performs a recursive depth-first exploration of the file system.
       *
       * @param[in] path            The location on disk to scan.
       * @param[in] fileNode        The TreeNode in Tree to append newly discoved files to.
       */
      void ProcessDirectory(
         const std::experimental::filesystem::path& path,
         TreeNode<VizNode>& fileNode);

      /**
       * @brief Helper function to facilitate exception-free iteration over a directory.
       *
       * @param[in] itr             Reference to the iterator to iterate over.
       * @param[in] treeNode        The TreeNode to append the contents of the directory to.
       */
      inline void IterateOverDirectoryAndScan(
         std::experimental::filesystem::directory_iterator& itr,
         TreeNode<VizNode>& treeNode) noexcept;

      std::shared_ptr<Tree<VizNode>> CreateTreeAndRootNode();

      DriveScanningParameters m_parameters;

      ScanningProgress& m_progress;
};

#endif // SCANNINGWORKER_H
