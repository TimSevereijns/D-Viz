#include "mainModel.h"

#include "Visualizations/squarifiedTreemap.h"
#include "Utilities/scopeExit.hpp"
#include "Windows/mainWindow.h"

#include <sstream>
#include <utility>

#include <QCursor>

MainModel::MainModel()
{
}

bool MainModel::HasVisualizationBeenLoaded() const
{
   return m_treeMap != nullptr;
}

void MainModel::GenerateNewVisualization(VisualizationParameters& parameters)
{
   // @todo: Do the parameters need to be passed in?

   if (parameters.rootDirectory.empty())
   {
      return;
   }

   if (!HasVisualizationBeenLoaded() || parameters.forceNewScan)
   {
      m_treeMap.reset(new SquarifiedTreeMap{ parameters });
      m_mainWindow->ScanDrive(parameters);
   }
}

const TreeNode<VizNode>* const MainModel::GetSelectedNode() const
{
   return m_selectedNode;
}

Tree<VizNode>& MainModel::GetTree()
{
   return m_treeMap->GetTree();
}

const Tree<VizNode>& MainModel::GetTree() const
{
   return m_treeMap->GetTree();
}

const VisualizationParameters& MainModel::GetVisualizationParameters() const
{
   return m_visualizationParameters;
}

void MainModel::SetVisualizationParameters(const VisualizationParameters& parameters)
{
   m_visualizationParameters = parameters;
}

const std::vector<const TreeNode<VizNode>*> MainModel::GetHighlightedNodes() const
{
   return m_highlightedNodes;
}

void MainModel::SetView(MainWindow* window)
{
   assert(m_mainWindow == nullptr);
   assert(window);

   m_mainWindow = window;
}

void MainModel::ParseResults(const std::shared_ptr<Tree<VizNode> >& results)
{
   m_treeMap->Parse(results);
}

void MainModel::UpdateBoundingBoxes()
{
   m_treeMap->UpdateBoundingBoxes();
}


