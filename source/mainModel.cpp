#include "mainModel.h"

#include "Viewport/glCanvas.h"
#include "Visualizations/squarifiedTreemap.h"
#include "Utilities/scopeExit.hpp"
#include "Windows/mainWindow.h"

#include <boost/algorithm/string/predicate.hpp>

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

void MainModel::ParseResults(const std::shared_ptr<Tree<VizNode>>& results)
{
   m_treeMap->Parse(results);
}

void MainModel::UpdateBoundingBoxes()
{
   m_treeMap->UpdateBoundingBoxes();
}

void MainModel::PaintHighlightNodes()
{
   //m_mainWindow->GetCanvas().HighlightSelectedNodes(m_highlightedNodes);
}

void MainModel::ClearHighlightedNodes()
{
   m_mainWindow->GetCanvas().RestoreHighlightedNodes(m_highlightedNodes);

   m_highlightedNodes.clear();
}

void MainModel::HighlightAncestors(const TreeNode<VizNode>& selectedNode)
{
   // @todo Add appropriate nodes to highlight vector
   // @todo Send reference to highlight vector over to the canvas.

   ClearHighlightedNodes();

   auto* currentNode = &selectedNode;
   do
   {
      m_highlightedNodes.emplace_back(currentNode);
      currentNode = currentNode->GetParent();
   }
   while (currentNode);

   PaintHighlightNodes();
}

void MainModel::HighlightDescendants(const TreeNode<VizNode>& selectedNode)
{
   ClearHighlightedNodes();

   std::for_each(
      Tree<VizNode>::LeafIterator{ &selectedNode },
      Tree<VizNode>::LeafIterator{ },
      [&] (Tree<VizNode>::const_reference node)
   {
      if ((m_visualizationParameters.onlyShowDirectories && node->file.type == FileType::REGULAR)
         || node->file.size < m_visualizationParameters.minimumFileSize)
      {
         return;
      }

      m_highlightedNodes.emplace_back(&node);
   });

   PaintHighlightNodes();
}

void MainModel::HighlightAllMatchingExtension(const TreeNode<VizNode>& selectedNode)
{
   ClearHighlightedNodes();

   std::for_each(
      Tree<VizNode>::LeafIterator{ GetTree().GetHead() },
      Tree<VizNode>::LeafIterator{ },
      [&] (Tree<VizNode>::const_reference node)
   {
      if ((m_visualizationParameters.onlyShowDirectories && node->file.type == FileType::REGULAR)
         || node->file.size < m_visualizationParameters.minimumFileSize
         || node->file.extension != selectedNode->file.extension)
      {
         return;
      }

      m_highlightedNodes.emplace_back(&node);
   });

   PaintHighlightNodes();
}

std::vector<const TreeNode<VizNode>*> MainModel::SearchTreeMap(
   bool shouldSearchFiles,
   bool shouldSearchDirectories)
{
   std::vector<const TreeNode<VizNode>*> results;

   const auto& searchQuery = m_mainWindow->GetSearchQuery();
   if (searchQuery.empty()
      || !HasVisualizationBeenLoaded()
      || (!shouldSearchFiles && !shouldSearchDirectories))
   {
      return { };
   }

   std::for_each(
      Tree<VizNode>::PostOrderIterator{ GetTree().GetHead() },
      Tree<VizNode>::PostOrderIterator{ },
      [&] (Tree<VizNode>::const_reference node)
   {
      if (node->file.size < m_visualizationParameters.minimumFileSize)
      {
         return;
      }

      if (!shouldSearchDirectories && node->file.type == FileType::DIRECTORY)
      {
         return;
      }

      if (!shouldSearchFiles && node->file.type == FileType::REGULAR)
      {
         return;
      }

      const auto fullFileName{ node->file.name + node->file.extension };
      if (!boost::icontains(fullFileName, searchQuery))
      {
         return;
      }

      results.emplace_back(&node);
   });

   return results;
}
