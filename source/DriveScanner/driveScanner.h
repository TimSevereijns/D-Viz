#ifndef DRIVESCANNER_H
#define DRIVESCANNER_H

#include <QObject>
#include <QThread>
#include <QTimer>

#include <cstdint>
#include <memory>
#include <string>

#include "../DataStructs/driveScanningParameters.h"
#include "../DataStructs/scanningprogress.hpp"

#include "scanningWorker.h"

/**
 * @brief The DriveScanner class uses a dedicate thread to scan the specified drive or part thereof.
 */
class DriveScanner : public QObject
{
   Q_OBJECT

   public:

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
       * @param[in] fileTree        The final tree representing the scanned drive (or a part
       *                            thereof).
       */
      void HandleCompletion(std::shared_ptr<Tree<VizNode>> fileTree);

      /**
       * @brief Handle the ScanningWorker::ProgressUpdate signal.
       *
       * @see ScanningWorker::ProgressUpdate
       */
      void HandleProgressUpdates();

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

      ScanningProgress m_progress;

      std::unique_ptr<QTimer> m_progressUpdateTimer{ nullptr };
};

#endif // DRIVESCANNER_H
