#include "driveScanner.h"

#include "../DataStructs/precisePoint.h"

#include <iostream>

#include <QMessageBox>

void DriveScanner::HandleProgressUpdates(
   const std::uintmax_t filesScanned,
   const std::uintmax_t numberOfBytesProcessed)
{
   m_parameters.onProgressUpdateCallback(filesScanned, numberOfBytesProcessed);
}

void DriveScanner::HandleCompletion(
   const std::uintmax_t filesScanned,
   std::shared_ptr<Tree<VizNode>> fileTree)
{
   m_parameters.onScanCompletedCallback(filesScanned, fileTree);
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
   m_parameters = parameters;

   QThread* thread = new QThread;
   ScanningWorker* worker = new ScanningWorker{ m_parameters };
   worker->moveToThread(thread);

   connect(worker, SIGNAL(Finished(const std::uintmax_t, std::shared_ptr<Tree<VizNode>>)),
      this, SLOT(HandleCompletion(const std::uintmax_t, std::shared_ptr<Tree<VizNode>>)));

   connect(worker, SIGNAL(ProgressUpdate(const std::uintmax_t, const std::uintmax_t)),
      this, SLOT(HandleProgressUpdates(const std::uintmax_t, const std::uintmax_t)));

   connect(worker, SIGNAL(ShowMessageBox(const QString&)),
      this, SLOT(HandleMessageBox(const QString&)), Qt::BlockingQueuedConnection);

   connect(worker, SIGNAL(Finished(const std::uintmax_t, std::shared_ptr<Tree<VizNode>>)),
      worker, SLOT(deleteLater()));

   connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
   connect(thread, SIGNAL(started()), worker, SLOT(Start()));

   thread->start();
}
