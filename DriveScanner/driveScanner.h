#ifndef DRIVESCANNER_H
#define DRIVESCANNER_H

#include <QObject>
#include <QThread>

#include <cstdint>
#include <memory>
#include <string>

#include "../DataStructs/driveScanningParameters.h"

#include "scanningWorker.h"

/**
 * @brief The DriveScanner class uses a dedicate thread to scan the specified drive or part thereof.
 */
class DriveScanner : public QObject
{
   Q_OBJECT

   public:
      ~DriveScanner();

      /**
       * @brief StartScanning kicks off the drive scanning process in a separate thread using the
       * specified parameters.
       *
       * @param[in] parameters      @see DriveScanningParameters
       */
      void StartScanning(const DriveScanningParameters& parameters);

   public slots:
      /**
       * @brief HandleCompletion is meant to handle the ScanningWorker::Finished signal.
       *
       * @see ScanningWorker::Finished
       */
      void HandleCompletion(
         const std::uintmax_t filesScanned,
         std::shared_ptr<Tree<VizNode>> fileTree);

      /**
       * @brief HandleProgressUpdates is meant to handle the ScanningWorker::ProgressUpdate
       * signal.
       *
       * @see ScanningWorker::ProgressUpdate
       */
      void HandleProgressUpdates(const std::uintmax_t filesScanned);

      /**
       * @brief HandleMessageBox is meant to handle the ScanningWorker::ShowMessageBox signal.
       *
       * @see ScanningWorker::ShowMessageBox
       */
      void HandleMessageBox(const QString& message);

   private:
      DriveScanningParameters m_parameters;
};

#endif // DRIVESCANNER_H
