#include "controller.h"

#include "constants.h"
#include "Viewport/glCanvas.h"
#include "Visualizations/squarifiedTreemap.h"
#include "Utilities/scopeExit.hpp"
#include "Windows/mainWindow.h"

#include <boost/algorithm/string/predicate.hpp>

#include <sstream>
#include <utility>

#include <QCursor>

#include <ShlObj.h>
#include <Objbase.h>

namespace
{
   const auto* const BYTES_READOUT_STRING = L" bytes";
}

Controller::Controller()
{
}

bool Controller::HasVisualizationBeenLoaded() const
{
   return m_treeMap != nullptr;
}

void Controller::GenerateNewVisualization(VisualizationParameters& parameters)
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

const TreeNode<VizNode>* const Controller::GetSelectedNode() const
{
   return m_selectedNode;
}

Tree<VizNode>& Controller::GetTree()
{
   return m_treeMap->GetTree();
}

const Tree<VizNode>& Controller::GetTree() const
{
   return m_treeMap->GetTree();
}

const VisualizationParameters& Controller::GetVisualizationParameters() const
{
   return m_visualizationParameters;
}

void Controller::SetVisualizationParameters(const VisualizationParameters& parameters)
{
   m_visualizationParameters = parameters;
}

const std::vector<const TreeNode<VizNode>*> Controller::GetHighlightedNodes() const
{
   return m_highlightedNodes;
}

void Controller::SetView(MainWindow* window)
{
   assert(m_mainWindow == nullptr);
   assert(window);

   m_mainWindow = window;
}

void Controller::ParseResults(const std::shared_ptr<Tree<VizNode>>& results)
{
   m_treeMap->Parse(results);
}

void Controller::UpdateBoundingBoxes()
{
   m_treeMap->UpdateBoundingBoxes();
}

void Controller::SelectNode(const TreeNode<VizNode>* const node)
{
   if (!node)
   {
      return;
   }

   ClearHighlightedNodes();

   const auto fileSize = node->GetData().file.size;
   assert(fileSize > 0);

   const auto sizeAndUnits = Controller::ConvertFileSizeToMostAppropriateUnits(fileSize);
   const auto isInBytes = (sizeAndUnits.second == BYTES_READOUT_STRING);

   std::wstringstream message;
   message.imbue(std::locale{ "" });
   message.precision(isInBytes ? 0 : 2);
   message
      << Controller::ResolveCompleteFilePath(*node)
      << L"  |  "
      << std::fixed
      << sizeAndUnits.first
      << sizeAndUnits.second;

   assert(message.str().size() > 0);
   m_mainWindow->SetStatusBarMessage(message.str());

   m_mainWindow->GetCanvas().SelectNode(node);
}

void Controller::PrintMetadataToStatusBar(const uint32_t blockCount)
{
   std::wstringstream message;
   message.imbue(std::locale(""));
   message
      << std::fixed
      << blockCount * Block::VERTICES_PER_BLOCK
      << L" vertices, representing "
      << blockCount
      << L" files.";

   m_mainWindow->SetStatusBarMessage(message.str());
}

void Controller::PaintHighlightNodes()
{
   if (m_highlightedNodes.empty())
   {
      return;
   }

   m_mainWindow->GetCanvas().HighlightSelectedNodes(m_highlightedNodes);

   std::uintmax_t selectionSizeInBytes{ 0 };
   for (const auto* const node : m_highlightedNodes)
   {
      selectionSizeInBytes += node->GetData().file.size;
   }

   const auto sizeAndUnits = Controller::ConvertFileSizeToMostAppropriateUnits(selectionSizeInBytes);
   const auto isInBytes = (sizeAndUnits.second == BYTES_READOUT_STRING);

   std::wstringstream message;
   message.imbue(std::locale{ "" });
   message.precision(isInBytes ? 0 : 2);
   message
      << L"Highlighting "
      << m_highlightedNodes.size()
      << (m_highlightedNodes.size() == 1 ? L" node" : L" nodes")
      << L", representing "
      << std::fixed
      << sizeAndUnits.first
      << sizeAndUnits.second;

   assert(message.str().size() > 0);
   m_mainWindow->SetStatusBarMessage(message.str());
}

void Controller::ClearHighlightedNodes()
{
   m_mainWindow->GetCanvas().RestoreHighlightedNodes(m_highlightedNodes);

   m_highlightedNodes.clear();
}

void Controller::HighlightAncestors(const TreeNode<VizNode>& node)
{
   ClearHighlightedNodes();

   auto* currentNode = &node;
   do
   {
      m_highlightedNodes.emplace_back(currentNode);
      currentNode = currentNode->GetParent();
   }
   while (currentNode);

   PaintHighlightNodes();
}

void Controller::HighlightDescendants(const TreeNode<VizNode>& node)
{
   ClearHighlightedNodes();

   std::for_each(
      Tree<VizNode>::LeafIterator{ &node },
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

void Controller::HighlightAllMatchingExtension(const TreeNode<VizNode>& targetNode)
{
   ClearHighlightedNodes();

   std::for_each(
      Tree<VizNode>::LeafIterator{ GetTree().GetHead() },
      Tree<VizNode>::LeafIterator{ },
      [&] (Tree<VizNode>::const_reference node)
   {
      if ((m_visualizationParameters.onlyShowDirectories && node->file.type == FileType::REGULAR)
         || node->file.size < m_visualizationParameters.minimumFileSize
         || node->file.extension != targetNode->file.extension)
      {
         return;
      }

      m_highlightedNodes.emplace_back(&node);
   });

   PaintHighlightNodes();
}

std::vector<const TreeNode<VizNode>*> Controller::SearchTreeMap(
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

std::pair<double, std::wstring> Controller::ConvertFileSizeToMostAppropriateUnits(
   double sizeInBytes)
{
   if (sizeInBytes < Constants::FileSize::ONE_KIBIBYTE)
   {
      return std::make_pair<double, std::wstring>(std::move(sizeInBytes), BYTES_READOUT_STRING);
   }

   if (sizeInBytes < Constants::FileSize::ONE_MEBIBYTE)
   {
      return std::make_pair<double, std::wstring>(
         sizeInBytes / Constants::FileSize::ONE_KIBIBYTE, L" KiB");
   }

   if (sizeInBytes < Constants::FileSize::ONE_GIBIBYTE)
   {
      return std::make_pair<double, std::wstring>(
         sizeInBytes / Constants::FileSize::ONE_MEBIBYTE, L" MiB");
   }

   if (sizeInBytes < Constants::FileSize::ONE_TEBIBYTE)
   {
      return std::make_pair<double, std::wstring>(
         sizeInBytes / Constants::FileSize::ONE_GIBIBYTE, L" GiB");
   }

   return std::make_pair<double, std::wstring>(
            sizeInBytes / Constants::FileSize::ONE_TEBIBYTE, L" TiB");
}

std::wstring Controller::ResolveCompleteFilePath(const TreeNode<VizNode>& node)
{
   std::vector<std::wstring> reversePath;
   reversePath.reserve(Tree<VizNode>::Depth(node));
   reversePath.emplace_back(node->file.name);

   const auto* currentNode = &node;

   while (currentNode->GetParent())
   {
      currentNode = currentNode->GetParent();
      reversePath.emplace_back(currentNode->GetData().file.name);
   }

   const auto completePath = std::accumulate(std::rbegin(reversePath), std::rend(reversePath),
      std::wstring{ }, [] (const std::wstring& path, const std::wstring& file)
   {
      return path + (!path.empty() ? L"/" : L"") + file;
   });

   assert(completePath.size() > 0);
   return completePath + node->file.extension;
}

void Controller::ShowInFileExplorer(const TreeNode<VizNode>& node)
{
   CoInitializeEx(NULL, COINIT_MULTITHREADED);
   ON_SCOPE_EXIT noexcept { CoUninitialize(); };

   std::wstring filePath = Controller::ResolveCompleteFilePath(node);
   std::replace(std::begin(filePath), std::end(filePath), L'/', L'\\');

   ITEMIDLIST __unaligned * idList = ILCreateFromPath(filePath.c_str());
   if (idList)
   {
      SHOpenFolderAndSelectItems(idList, 0, 0, 0);
      ILFree(idList);
   }
}
