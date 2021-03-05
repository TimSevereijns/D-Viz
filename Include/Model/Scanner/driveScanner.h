#ifndef DRIVESCANNER_H
#define DRIVESCANNER_H

#include <QObject>
#include <QThread>
#include <QTimer>

#include <cstdint>
#include <memory>
#include <string>

#include "Model/Scanner/scanningParameters.h"
#include "Model/Scanner/scanningProgress.h"
#include "Model/Scanner/scanningWorker.h"

/**
 * @brief Walks the filesystem using a dedicate thread to build a tree that represents said file
 * system.
 */
class DriveScanner final : public QObject
{
    Q_OBJECT

  public:
    /**
     * @brief Kicks off the drive scanning process in a separate thread using the specified
     * parameters.
     *
     * @param[in] parameters      @see DriveScanningParameters
     */
    void StartScanning(const ScanningParameters& parameters);

    /**
     * @brief Halts progress reporting for an active scan.
     */
    void StopProgressReporting();

    /**
     * @brief Stops scanning.
     */
    void StopScanning();

    /**
     * @returns True if the scanner is running.
     */
    bool IsActive() const;

  public slots:

    /**
     * @brief Handles the ScanningWorker::Finished signal.
     *
     * @see ScanningWorker::Finished
     *
     * @param[in] fileTree        The final tree representing the scanned drive (or a part thereof).
     */
    void HandleCompletion(const std::shared_ptr<Tree<VizBlock>>& fileTree);

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

  signals:

    void Finished();

  private:
    ScanningParameters m_parameters;

    ScanningProgress m_progress;

    std::atomic<bool> m_cancellationToken;

    bool m_isActive{ false };

    std::unique_ptr<QTimer> m_progressUpdateTimer{ nullptr };
};

#endif // DRIVESCANNER_H
