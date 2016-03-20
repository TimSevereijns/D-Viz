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
       * @brief Kicks off the drive scanning process in a separate thread using the specified
       * parameters.
       *
       * @param[in] parameters      @see DriveScanningParameters
       */
      void StartScanning(const DriveScanningParameters& parameters);

   public slots:
      /**
       * @brief Handles the ScanningWorker::Finished signal.
       *
       * @see ScanningWorker::Finished
       *
       * @param[in] filesScanned    Number of files scanned.
       * @param[in] fileTree        The final tree representing the scanned drive (or a part
       *                            thereof).
       */
      void HandleCompletion(
         const std::uintmax_t filesScanned,
         std::shared_ptr<Tree<VizNode>> fileTree);

      /**
       * @brief Handle the ScanningWorker::ProgressUpdate signal.
       *
       * @see ScanningWorker::ProgressUpdate
       *
       * @param[in] filesScanned    The number of files scanned.
       */
      void HandleProgressUpdates(const std::uintmax_t filesScanned);

      /**
       * @brief Handle the ScanningWorker::ShowMessageBox signal.
       *
       * @see ScanningWorker::ShowMessageBox
       *
       * @param[in] message         The message to be displayed to the user.
       */
      void HandleMessageBox(const QString& message);

   private:
      DriveScanningParameters m_parameters;
};

#endif // DRIVESCANNER_H
