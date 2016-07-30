#include "mainModel.h"

#include "Visualizations/squarifiedTreemap.h"
#include "Utilities/scopeExit.hpp"

#include <sstream>
#include <utility>

#include <QCursor>

MainModel::MainModel()
{
}

bool MainModel::HasVisualizationBeenLoaded() const
{
   return m_theVisualization != nullptr;
}

void MainModel::GenerateNewVisualization(VisualizationParameters& parameters)
{
   if (parameters.rootDirectory.empty())
   {
      return;
   }

   if (!HasVisualizationBeenLoaded() || parameters.forceNewScan)
   {
      m_theVisualization.reset(new SquarifiedTreeMap{ parameters });
      ScanDrive(parameters);
   }
}

const TreeNode<VizNode>* const MainModel::GetSelectedNode() const
{
   return m_selectedNode;
}

Tree<VizNode>&MainModel::GetTree()
{
   return m_theVisualization->GetTree();
}

const Tree<VizNode>&MainModel::GetTree() const
{
   return m_theVisualization->GetTree();
}

const VisualizationParameters&MainModel::GetVisualizationParameters() const
{
   return m_visualizationParameters;
}

const std::vector<const TreeNode<VizNode>*> MainModel::GetHighlightedNodes() const
{
   return m_highlightedNodes;
}

void MainModel::ScanDrive(VisualizationParameters& vizParameters)
{
   const auto progressHandler =
      [&] (const std::uintmax_t numberOfFilesScanned)
   {
      std::wstringstream message;
      message.imbue(std::locale{ "" });
      message << std::fixed << L"Files Scanned: " << numberOfFilesScanned;
      //m_mainWindow->SetStatusBarMessage(message.str());
   };

   const auto completionHandler =
      [&, vizParameters] (const std::uintmax_t numberOfFilesScanned,
      std::shared_ptr<Tree<VizNode>> fileTree) mutable
   {
//      QCursor previousCursor = cursor();
//      setCursor(Qt::WaitCursor);
//      ON_SCOPE_EXIT{ setCursor(previousCursor); };
//      QApplication::processEvents();

      std::wstringstream message;
      message.imbue(std::locale{ "" });
      message << std::fixed << L"Total Files Scanned: " << numberOfFilesScanned;
      //m_mainWindow->SetStatusBarMessage(message.str());

      //AskUserToLimitFileSize(numberOfFilesScanned, vizParameters);

      m_theVisualization->Parse(fileTree);
      m_theVisualization->UpdateBoundingBoxes();

      //ReloadVisualization(vizParameters);
   };

   const DriveScanningParameters scanningParameters
   {
      vizParameters.rootDirectory,
      progressHandler,
      completionHandler
   };

   m_scanner.StartScanning(scanningParameters);
}
