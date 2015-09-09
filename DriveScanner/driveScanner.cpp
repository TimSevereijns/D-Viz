#include "driveScanner.h"

#include <iostream>

DriveScanner::DriveScanner(QObject* parent)
   : QObject(parent)
{
}

void DriveScanner::HandleProgressUpdates(const std::uintmax_t filesScanned)
{
   std::wcout << L"Progress Update: " << filesScanned << L" files scanned..." << std::endl;
}

void DriveScanner::HandleErrors(const std::wstring message)
{
   std::wcout << L"Error: " << message << std::endl;
}

void DriveScanner::StartScanning()
{
   m_thread.reset(new QThread);
   m_worker.reset(new ScanningWorker());
   m_worker->moveToThread(m_thread.get());

   connect(m_worker.get(), SIGNAL(Error(std::wstring)),
           this, SLOT(HandleErrors(std::wstring)));

   connect(m_worker, SIGNAL(Finished(std::uintmax_t)),
           m_thread, SLOT(HandleCompletion(std::uintmax_t)));

   //connect(m_worker, SIGNAL(finished()), m_worker, SLOT(deleteLater()));

   connect(m_thread.get(), SIGNAL(finished()), m_thread.get(), SLOT(deleteLater()));
   connect(m_thread.get(), SIGNAL(started()), m_worker.get(), SLOT(Start()));

   m_thread->start();
}

