#include "driveScanner.h"

#include <iostream>

DriveScanner::DriveScanner()
   : QObject(),
     m_theTree(nullptr),
     m_thread(nullptr),
     m_worker(nullptr)
{
}

DriveScanner::DriveScanner(const DriveScannerParameters& parameters)
   : QObject(),
     m_scanningParameters(parameters)
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

   FileInfo fileInfo{L"Dummy Root Node", ScanningWorker::SIZE_UNDEFINED, FILE_TYPE::DIRECTORY};
   VizNode rootNode{fileInfo, rootBlock};

   m_theTree = std::make_shared<Tree<VizNode>>(Tree<VizNode>(rootNode));

   m_thread.reset(new QThread);
   m_worker.reset(new ScanningWorker(m_theTree.get(), m_scanningParameters.m_path));
   m_worker->moveToThread(m_thread.get());

   connect(m_worker.get(), SIGNAL(Error(std::wstring)),
           this, SLOT(HandleErrors(std::wstring)));

   connect(m_worker.get(), SIGNAL(Finished(const std::uintmax_t)),
           this, SLOT(HandleCompletion(const std::uintmax_t)));

   connect(m_worker.get(), SIGNAL(ProgressUpdate(const std::uintmax_t)),
           this, SLOT(HandleProgressUpdates(std::uintmax_t)));

   connect(m_worker.get(), SIGNAL(Finished(const std::uintmax_t)),
           m_worker.get(), SLOT(deleteLater()));

   connect(m_thread.get(), SIGNAL(finished()), m_thread.get(), SLOT(deleteLater()));
   connect(m_thread.get(), SIGNAL(finished()), m_thread.get(), SLOT(deleteLater()));
   connect(m_thread.get(), SIGNAL(started()), m_worker.get(), SLOT(Start()));

   m_thread->start();
}

std::shared_ptr<Tree<VizNode> > DriveScanner::GetTree() const
{
   return m_theTree;
}
