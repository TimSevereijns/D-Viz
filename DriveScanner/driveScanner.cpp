#include "driveScanner.h"

#include <iostream>

DriveScanner::DriveScanner()
   : QObject(),
     m_theTree(nullptr)
{
}

DriveScanner::~DriveScanner()
{
   std::cout << "Drive Scanner is dead..." << std::endl;
}

void DriveScanner::SetParameters(const DriveScannerParameters& parameters)
{
   m_scanningParameters = parameters;
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
   const Block rootBlock
   {
      QVector3D(0, 0, 0),
      Visualization::ROOT_BLOCK_WIDTH,
      Visualization::BLOCK_HEIGHT,
      Visualization::ROOT_BLOCK_DEPTH
   };

   const FileInfo fileInfo
   {
      m_scanningParameters.m_path,
      ScanningWorker::SIZE_UNDEFINED,
      FILE_TYPE::DIRECTORY
   };

   const VizNode rootNode
   {
      fileInfo,
      rootBlock
   };

   m_theTree = std::make_shared<Tree<VizNode>>(Tree<VizNode>(rootNode));

   QThread* thread = new QThread;
   ScanningWorker* worker = new ScanningWorker(m_theTree, m_scanningParameters.m_path);
   worker->moveToThread(thread);

   connect(worker, SIGNAL(Error(std::wstring)),
      this, SLOT(HandleErrors(std::wstring)));

   connect(worker, SIGNAL(Finished(const std::uintmax_t)),
      this, SLOT(HandleCompletion(const std::uintmax_t)));

   connect(worker, SIGNAL(ProgressUpdate(const std::uintmax_t)),
      this, SLOT(HandleProgressUpdates(std::uintmax_t)));

   connect(worker, SIGNAL(Finished(const std::uintmax_t)),
      worker, SLOT(deleteLater()));

   connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
   connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
   connect(thread, SIGNAL(started()), worker, SLOT(Start()));

   thread->start();
}

std::shared_ptr<Tree<VizNode>> DriveScanner::GetTree() const
{
   return m_theTree;
}
