#include "driveScanner.h"

#include "../DataStructs/precisePoint.h"

#include <iostream>

#include <QMessageBox>

void DriveScanner::HandleProgressUpdates()
{
   const auto filesScanned = m_progress.filesScanned.load();
   const auto numberOfBytesProcessed = m_progress.numberOfBytesProcessed.load();

   m_parameters.onProgressUpdateCallback(filesScanned, numberOfBytesProcessed);
}

void DriveScanner::HandleCompletion(std::shared_ptr<Tree<VizNode>> fileTree)
{
   const auto filesScanned = m_progress.filesScanned.load();
   const auto numberOfBytesProcessed = m_progress.numberOfBytesProcessed.load();

   m_progressUpdateTimer->stop();

   m_parameters.onScanCompletedCallback(filesScanned, numberOfBytesProcessed, fileTree);
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
   connect(m_progressUpdateTimer.get(), SIGNAL(timeout()), this, SLOT(HandleProgressUpdates()));

   m_parameters = parameters;

   QThread* thread = new QThread;
   ScanningWorker* worker = new ScanningWorker{ m_parameters, m_progress };
   worker->moveToThread(thread);

   m_progressUpdateTimer->start(250);

   connect(worker, SIGNAL(Finished(std::shared_ptr<Tree<VizNode>>)),
      this, SLOT(HandleCompletion(std::shared_ptr<Tree<VizNode>>)));

   connect(worker, SIGNAL(Finished(std::shared_ptr<Tree<VizNode>>)),
      worker, SLOT(deleteLater()));

   connect(worker, SIGNAL(ProgressUpdate()),
      this, SLOT(HandleProgressUpdates()));

   connect(worker, SIGNAL(ShowMessageBox(const QString&)),
      this, SLOT(HandleMessageBox(const QString&)), Qt::BlockingQueuedConnection);

   connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
   connect(thread, SIGNAL(started()), worker, SLOT(Start()));

   thread->start();
}
