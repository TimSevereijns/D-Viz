#ifndef SCANNINGWORKER_H
#define SCANNINGWORKER_H

#include <boost/filesystem.hpp>

#include <QObject>
#include <QVector>
#include <QVector3D>

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

#include "../DataStructs/block.h"
#include "../DataStructs/driveScanningParameters.h"
#include "../DataStructs/fileInfo.h"
#include "../DataStructs/vizNode.h"

#include "../Visualizations/visualization.h"

#include "../tree.h"

/**
 * @brief The ScanningWorker class
 */
class ScanningWorker : public QObject
{
   Q_OBJECT

   public:
      static const std::uintmax_t SIZE_UNDEFINED;

      explicit ScanningWorker(const DriveScanningParameters& parameters);
      ~ScanningWorker();

   public slots:
      /**
       * @brief Start kicks off the drive scanning process. As part of the scanning process, the
       * ProgressUpdate signal will be fired to signal progress updates, and the Finish signal will
       * be fired once the scanning process completes successfully.
       *
       * @see Finished
       * @see ProgressUpdate
       */
      void Start();

   signals:
      /**
       * @brief Finished signals that the drive scanning has finished.
       *
       * @param[in] filesScanned    The total number of files that were scanned.
       * @param[in] fileTree        A pointer to the final tree representing the scanned drive.
       */
      void Finished(const std::uintmax_t filesScanned, std::shared_ptr<Tree<VizNode>> fileTree);

      /**
       * @brief ProgressUpdate signals drive scanning progress updates.
       *
       * @param[in] filesScanned    The number of files scanned so far.
       */
      void ProgressUpdate(const std::uintmax_t filesScanned);

      /**
       * @brief ShowMessageBox allows for cross-thread signaling show the user a standard message
       * box.
       *
       * @param[in] message         The message to be shown in the message box.
       */
      void ShowMessageBox(const QString& message);

   private:
      /**
       * Max path length in Windows is 260 characters, so if that includes slashes, then the maximum
       * depth of a directory or file is no more than 130, or so. Given that the default stack size
       * in MSVC is 1MB, and I only pass in references, this recursive version may be fine---maybe!
       */
      void ScanRecursively(const boost::filesystem::path& path, TreeNode<VizNode>& fileNode);

      std::shared_ptr<Tree<VizNode>> CreateRootNode();

      std::uintmax_t m_filesScanned;

      std::chrono::duration<double> m_scanningTime;
      std::chrono::high_resolution_clock::time_point m_lastProgressUpdate;

      DriveScanningParameters m_parameters;
};

#endif // SCANNINGWORKER_H
