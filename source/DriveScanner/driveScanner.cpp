#include "driveScanner.h"

#include "../DataStructs/precisePoint.h"

#include <iostream>

#include <QMessageBox>

void DriveScanner::HandleProgressUpdates()
{
   m_parameters.onProgressUpdateCallback(m_progress);
}

void DriveScanner::HandleCompletion(std::shared_ptr<Tree<VizFile>> fileTree)
{
   m_progressUpdateTimer->stop();

   m_parameters.onScanCompletedCallback(m_progress, fileTree);
}

void DriveScanner::HandleMessageBox(const QString& message)
{
   QMessageBox messageBox;
   messageBox.setText(message);
   messageBox.setIcon(QMessageBox::Warning);
   messageBox.setStandardButtons(QMessageBox::Ok);
   messageBox.exec();
}

void DriveScanner::StartScanning(const DriveScanningParameters& parameters)
{
   m_progress.Reset();
   m_progressUpdateTimer = std::make_unique<QTimer>(this);
   connect(m_progressUpdateTimer.get(), &QTimer::timeout,
      this, &DriveScanner::HandleProgressUpdates);

   m_parameters = parameters;

   QThread* thread = new QThread;
   ScanningWorker* worker = new ScanningWorker{ m_parameters, m_progress };
   worker->moveToThread(thread);

   m_progressUpdateTimer->start(250);

   connect(worker, &ScanningWorker::Finished, this, &DriveScanner::HandleCompletion);
   connect(worker, &ScanningWorker::Finished, worker, &ScanningWorker::deleteLater);
   connect(worker, &ScanningWorker::ProgressUpdate, this, &DriveScanner::HandleProgressUpdates);
   connect(worker, &ScanningWorker::ShowMessageBox,
      this, &DriveScanner::HandleMessageBox, Qt::BlockingQueuedConnection);

   connect(thread, &QThread::finished, thread, &QThread::deleteLater);
   connect(thread, &QThread::started, worker, &ScanningWorker::Start);

   thread->start();
}
