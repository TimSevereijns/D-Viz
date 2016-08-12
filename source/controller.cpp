#include "controller.h"

#include "constants.h"
#include "Utilities/scopeExit.hpp"
#include "Viewport/glCanvas.h"
#include "Visualizations/squarifiedTreemap.h"
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

bool Controller::HasVisualizationBeenLoaded() const
{
   return m_treeMap != nullptr;
}

void Controller::GenerateNewVisualization()
{
   if (m_visualizationParameters.rootDirectory.empty())
   {
      return;
   }

   if (!HasVisualizationBeenLoaded() || m_visualizationParameters.forceNewScan)
   {
      m_treeMap.reset(new SquarifiedTreeMap{ m_visualizationParameters });
      m_mainWindow->ScanDrive(m_visualizationParameters);
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

const std::vector<const TreeNode<VizNode>*>& Controller::GetHighlightedNodes() const
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

   ClearSelectedNode();
   ClearHighlightedNodes();

   m_selectedNode = node;

   const auto fileSize = node->GetData().file.size;
   assert(fileSize > 0);

   const auto sizeAndUnits = Controller::ConvertFileSizeToAppropriateUnits(fileSize);
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

void Controller::SelectNodeViaRay(
   const Camera& camera,
   const Qt3DCore::QRay3D& ray)
{
   if (!HasVisualizationBeenLoaded())
   {
      return;
   }

   assert(m_treeMap);

   // @todo Remove the camera from the parameter list; just pass in a point...
   const auto* node = m_treeMap->FindNearestIntersection(camera, ray, m_visualizationParameters);
   if (node)
   {
      SelectNode(node);
   }
   else
   {
      ClearSelectedNode();
      ClearHighlightedNodes();

      const auto nodeCount = GetTree().Size();
      PrintMetadataToStatusBar(static_cast<uint32_t>(nodeCount));
   }
}

void Controller::PrintMetadataToStatusBar(const uint32_t nodeCount)
{
   std::wstringstream message;
   message.imbue(std::locale(""));
   message
      << std::fixed
      << nodeCount * Block::VERTICES_PER_BLOCK
      << L" vertices, representing "
      << nodeCount
      << L" files.";

   m_mainWindow->SetStatusBarMessage(message.str());
}

void Controller::PrintSelectionDetailsToStatusBar()
{
   std::uintmax_t selectionSizeInBytes{ 0 };
   for (const auto* const node : m_highlightedNodes)
   {
      selectionSizeInBytes += node->GetData().file.size;
   }

   const auto sizeAndUnits = Controller::ConvertFileSizeToAppropriateUnits(selectionSizeInBytes);
   const auto isInBytes = (sizeAndUnits.second == BYTES_READOUT_STRING);

   std::wstringstream message;
   message.imbue(std::locale{ "" });
   message.precision(isInBytes ? 0 : 2);
   message
      << L"Highlighted "
      << m_highlightedNodes.size()
      << (m_highlightedNodes.size() == 1 ? L" node" : L" nodes")
      << L", representing "
      << std::fixed
      << sizeAndUnits.first
      << sizeAndUnits.second;

   m_mainWindow->SetStatusBarMessage(message.str());
}

void Controller::PaintHighlightedNodes()
{
   if (m_highlightedNodes.empty())
   {
      return;
   }

   m_mainWindow->GetCanvas().HighlightSelectedNodes(m_highlightedNodes);

   PrintSelectionDetailsToStatusBar();
}

void Controller::ClearSelectedNode()
{
   m_mainWindow->GetCanvas().RestoreSelectedNode();

   m_selectedNode = nullptr;
}

void Controller::ClearHighlightedNodes()
{
   m_mainWindow->GetCanvas().RestoreHighlightedNodes(m_highlightedNodes);

   m_highlightedNodes.clear();
}

template<typename LambdaType>
void Controller::Highlight(
   const LambdaType& nodeSelector,
   bool shouldClearSelectedNode,
   bool shouldClearPreviouslyHighlightedNodes)
{
   if (shouldClearPreviouslyHighlightedNodes)
   {
      ClearHighlightedNodes();
   }

   if (shouldClearSelectedNode)
   {
      ClearSelectedNode();
   }

   nodeSelector();

   PaintHighlightedNodes();
}

void Controller::HighlightAncestors(const TreeNode<VizNode>& node)
{
   const auto selector = [&]
   {
      auto* currentNode = &node;
      do
      {
         m_highlightedNodes.emplace_back(currentNode);
         currentNode = currentNode->GetParent();
      }
      while (currentNode);
   };

   Highlight(
      selector,
      /* shouldClearSelectedNode = */ false,
      /* shouldClearPreviouslyHighlightedNodes = */ true);
}

void Controller::HighlightDescendants(const TreeNode<VizNode>& node)
{
   const auto selector = [&]
   {
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
   };

   Highlight(
      selector,
      /* shouldClearSelectedNode = */ false,
      /* shouldClearPreviouslyHighlightedNodes = */ true);
}

void Controller::HighlightAllMatchingExtension(const TreeNode<VizNode>& targetNode)
{
   const auto selector = [&]
   {
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
   };

   Highlight(
      selector,
      /* shouldClearSelectedNode = */ false,
      /* shouldClearPreviouslyHighlightedNodes = */ true);
}

void Controller::SearchTreeMap(
   const std::wstring& searchQuery,
   bool shouldSearchFiles,
   bool shouldSearchDirectories)
{
   if (searchQuery.empty()
      || !HasVisualizationBeenLoaded()
      || (!shouldSearchFiles && !shouldSearchDirectories))
   {
      return;
   }

   const auto selector = [&]
   {
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

         m_highlightedNodes.emplace_back(&node);
      });
   };

   Highlight(
      selector,
      /* shouldClearSelectedNode = */ true,
      /* shouldClearPreviouslyHighlightedNodes = */ true);
}

std::pair<double, std::wstring> Controller::ConvertFileSizeToAppropriateUnits(
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
