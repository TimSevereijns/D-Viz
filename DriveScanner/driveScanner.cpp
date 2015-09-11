#include "driveScanner.h"

#include <iostream>

DriveScanner::DriveScanner(const DriveScannerParameters& parameters)
   : QObject(),
     m_scanningParameters(parameters)
{
}

void DriveScanner::HandleProgressUpdates(const std::uintmax_t filesScanned)
{
   m_scanningParameters.m_onProgressUpdateCallback(filesScanned);
}

void DriveScanner::HandleErrors(const std::wstring message)
{
   m_scanningParameters.m_onErrorCallback(message);
}

void DriveScanner::HandleCompletion(const std::uintmax_t filesScanned)
{
   m_scanningParameters.m_onScanCompletedCallback(filesScanned);
}

void DriveScanner::StartScanning()
{
   m_thread.reset(new QThread);
   m_worker.reset(new ScanningWorker());
   m_worker->moveToThread(m_thread.get());

   connect(m_worker.get(), SIGNAL(Error(std::wstring)),
           this, SLOT(HandleErrors(std::wstring)));

   connect(m_worker.get(), SIGNAL(Finished(std::uintmax_t)),
           m_thread.get(), SLOT(HandleCompletion(std::uintmax_t)));

   connect(m_worker.get(), SIGNAL(finished()), m_worker.get(), SLOT(deleteLater()));
   connect(m_thread.get(), SIGNAL(finished()), m_thread.get(), SLOT(deleteLater()));

   connect(m_thread.get(), SIGNAL(finished()), m_thread.get(), SLOT(deleteLater()));
   connect(m_thread.get(), SIGNAL(started()), m_worker.get(), SLOT(Start()));

   m_thread->start();
}

