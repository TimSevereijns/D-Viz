#include "Model/Scanner/driveScanner.h"

#include "Model/precisePoint.h"

#include <QMessageBox>

void DriveScanner::HandleProgressUpdates()
{
    m_parameters.onProgressUpdateCallback(m_progress);
}

void DriveScanner::HandleCompletion(const std::shared_ptr<Tree<VizBlock>>& fileTree)
{
    m_progressUpdateTimer->stop();

    m_parameters.onScanCompletedCallback(m_progress, fileTree);

    m_isActive = false;
    emit Finished();
}

void DriveScanner::HandleMessageBox(const QString& message)
{
    QMessageBox messageBox;
    messageBox.setText(message);
    messageBox.setIcon(QMessageBox::Warning);
    messageBox.setStandardButtons(QMessageBox::Ok);
    messageBox.exec();
}

void DriveScanner::StartScanning(const ScanningParameters& parameters)
{
    m_progress.Reset();

    m_progressUpdateTimer = std::make_unique<QTimer>(this);
    connect(
        m_progressUpdateTimer.get(), &QTimer::timeout, this, &DriveScanner::HandleProgressUpdates);

    m_parameters = parameters;
    m_cancellationToken.store(false);

    auto* thread = new QThread;
    auto* worker = new ScanningWorker{ m_parameters, m_progress, m_cancellationToken };
    worker->moveToThread(thread);

    m_progressUpdateTimer->start(250);

    connect(worker, &ScanningWorker::Finished, this, &DriveScanner::HandleCompletion);
    connect(worker, &ScanningWorker::Finished, worker, &ScanningWorker::deleteLater);
    connect(worker, &ScanningWorker::ProgressUpdate, this, &DriveScanner::HandleProgressUpdates);
    connect(
        worker, &ScanningWorker::ShowMessageBox, this, &DriveScanner::HandleMessageBox,
        Qt::BlockingQueuedConnection);

    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(thread, &QThread::started, worker, &ScanningWorker::Start);

    m_isActive = true;
    thread->start();
}

void DriveScanner::StopScanning()
{
    m_cancellationToken.store(true);
    m_isActive = false;
}

bool DriveScanner::IsActive() const
{
    return m_isActive;
}

void DriveScanner::StopProgressReporting()
{
    if (m_progressUpdateTimer && m_progressUpdateTimer->isActive()) {
        m_progressUpdateTimer->stop();
    }
}
