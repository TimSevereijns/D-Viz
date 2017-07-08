#include "controller.h"

#include "constants.h"
#include "Utilities/scopeExit.hpp"
#include "Visualizations/squarifiedTreemap.h"
#include "Windows/mainWindow.h"

#include <ArenaAllocator/ArenaAllocator.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <spdlog/spdlog.h>
#include <Stopwatch/Stopwatch.hpp>

#include <algorithm>
#include <sstream>
#include <utility>

#include <QCursor>

#include <ShlObj.h>
#include <Objbase.h>

namespace
{
   const auto* const BYTES_READOUT_STRING = L" bytes";

   template<std::size_t ArenaSize = 512>
   using WideStackString =
      std::basic_string<
         wchar_t,
         std::char_traits<wchar_t>,
         ArenaAllocator<
            wchar_t,
            ArenaSize,
            alignof(wchar_t)
         >
      >;
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

const Tree<VizFile>::Node* const Controller::GetSelectedNode() const
{
   return m_selectedNode;
}

Tree<VizFile>& Controller::GetTree()
{
   return m_treeMap->GetTree();
}

const Tree<VizFile>& Controller::GetTree() const
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

const std::vector<const Tree<VizFile>::Node*>& Controller::GetHighlightedNodes() const
{
   return m_highlightedNodes;
}

void Controller::SetView(MainWindow* window)
{
   assert(m_mainWindow == nullptr);
   assert(window);

   m_mainWindow = window;
}

void Controller::ParseResults(const std::shared_ptr<Tree<VizFile>>& results)
{
   m_treeMap->Parse(results);
}

void Controller::UpdateBoundingBoxes()
{
   m_treeMap->UpdateBoundingBoxes();
}

void Controller::SelectNodeAndUpdateStatusBar(
   const Tree<VizFile>::Node* const node,
   const std::function<void (const Tree<VizFile>::Node* const)>& selectorCallback)
{
   if (!node)
   {
      return;
   }

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

   selectorCallback(node);
}

void Controller::SelectNodeViaRay(
   const Camera& camera,
   const Qt3DRender::QRay3D& ray,
   const std::function<void (std::vector<const Tree<VizFile>::Node*>&)>& deselectionCallback,
   const std::function<void (const Tree<VizFile>::Node* const)>& selectionCallback)
{
   if (!HasVisualizationBeenLoaded() || !IsUserAllowedToInteractWithModel())
   {
      return;
   }

   assert(m_treeMap);

   constexpr auto clearSelected{ true };
   ClearHighlightedNodes(deselectionCallback, clearSelected);

   // @todo Remove the camera from the parameter list; just pass in a point...
   const auto* node = m_treeMap->FindNearestIntersection(camera, ray, m_visualizationParameters);
   if (node)
   {
      SelectNodeAndUpdateStatusBar(node, selectionCallback);
   }
   else
   {
      // @todo Walking the entire tree isn't exactly efficient; find a better way...
      const auto nodeCount = std::count_if(
         Tree<VizFile>::LeafIterator{ m_treeMap->GetTree().GetRoot() },
         Tree<VizFile>::LeafIterator{ },
         [] (Tree<VizFile>::const_reference /*node*/)
      {
         return true;
      });

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

void Controller::AllowUserInteractionWithModel(bool allowInteraction)
{
   m_allowInteractionWithModel = allowInteraction;
}

bool Controller::IsUserAllowedToInteractWithModel() const
{
   return m_allowInteractionWithModel;
}

void Controller::ClearSelectedNode()
{
   m_selectedNode = nullptr;
}

void Controller::ClearHighlightedNodes(
   const std::function<void (std::vector<const Tree<VizFile>::Node*>&)>& callback,
   bool clearSelected)
{
   if (clearSelected && m_selectedNode)
   {
      m_highlightedNodes.emplace_back(m_selectedNode);
   }

   callback(m_highlightedNodes);
   m_highlightedNodes.clear();

   if (clearSelected && m_selectedNode)
   {
      m_selectedNode = nullptr;
   }
}

template<typename NodeSelectorType>
void Controller::ProcessSelection(
   const NodeSelectorType& nodeSelector,
   const std::function<void (std::vector<const Tree<VizFile>::Node*>&)>& callback)
{
   nodeSelector();

   callback(m_highlightedNodes);

   // @todo Consider making this part of the callback as well:
   PrintSelectionDetailsToStatusBar();
}

void Controller::HighlightAncestors(
   const Tree<VizFile>::Node& node,
   const std::function<void (std::vector<const Tree<VizFile>::Node*>&)>& callback)
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

   ProcessSelection(selector, callback);
}

void Controller::HighlightDescendants(
   const Tree<VizFile>::Node& node,
   const std::function<void (std::vector<const Tree<VizFile>::Node*>&)>& callback)
{
   const auto selector = [&]
   {
      std::for_each(
         Tree<VizFile>::LeafIterator{ &node },
         Tree<VizFile>::LeafIterator{ },
         [&] (Tree<VizFile>::const_reference node)
      {
         if ((m_visualizationParameters.onlyShowDirectories && node->file.type != FileType::DIRECTORY)
            || node->file.size < m_visualizationParameters.minimumFileSize)
         {
            return;
         }

         m_highlightedNodes.emplace_back(&node);
      });
   };

   ProcessSelection(selector, callback);
}

void Controller::HighlightAllMatchingExtensions(
   const Tree<VizFile>::Node& sampleNode,
   const std::function<void (std::vector<const Tree<VizFile>::Node*>&)>& callback)
{
   const auto selector = [&]
   {
      std::for_each(
         Tree<VizFile>::LeafIterator{ GetTree().GetRoot() },
         Tree<VizFile>::LeafIterator{ },
         [&] (Tree<VizFile>::const_reference node)
      {
         if ((m_visualizationParameters.onlyShowDirectories && node->file.type != FileType::DIRECTORY)
            || node->file.size < m_visualizationParameters.minimumFileSize
            || node->file.extension != sampleNode->file.extension)
         {
            return;
         }

         m_highlightedNodes.emplace_back(&node);
      });
   };

   ProcessSelection(selector, callback);
}

void Controller::SearchTreeMap(
   const std::wstring& searchQuery,
   const std::function<void (std::vector<const Tree<VizFile>::Node*>&)>& deselectionCallback,
   const std::function<void (std::vector<const Tree<VizFile>::Node*>&)>& selectionCallback,
   bool shouldSearchFiles,
   bool shouldSearchDirectories)
{
   if (searchQuery.empty()
      || !HasVisualizationBeenLoaded()
      || (!shouldSearchFiles && !shouldSearchDirectories))
   {
      return;
   }

   constexpr auto clearSelected{ true };
   ClearHighlightedNodes(deselectionCallback, clearSelected);

   const auto selector = [&]
   {
      // Using a stack allocated string (along with case sensitive comparison) appears to be about
      // 25-30% percent faster compared to a regular heap allocated string:
      // 212ms vs 316ms for ~750,000 files scanned on an old Intel Q9450.
      WideStackString<540>::allocator_type::arena_type stringArena{ };
      WideStackString<540> fullName{ std::move(stringArena) };
      fullName.resize(260); ///< Resize to prevent reallocation with later append operations.

      Stopwatch<std::chrono::milliseconds>([&] ()
      {
         std::for_each(
            Tree<VizFile>::PostOrderIterator{ GetTree().GetRoot() },
            Tree<VizFile>::PostOrderIterator{ },
            [&] (Tree<VizFile>::const_reference node)
         {
            const auto& file = node->file;

            if (file.size < m_visualizationParameters.minimumFileSize
               || (!shouldSearchDirectories && file.type == FileType::DIRECTORY)
               || (!shouldSearchFiles && file.type == FileType::REGULAR))
            {
               return;
            }

            fullName = file.name.data();
            fullName.append(file.extension.data());

            if (!boost::icontains(fullName, searchQuery))
            {
               return;
            }

            m_highlightedNodes.emplace_back(&node);
         });
      }, [] (const auto& elapsed, const auto& units) noexcept
      {
         spdlog::get(Constants::Logging::LOG_NAME)->info(
            fmt::format("Search Completed in: {} {}", elapsed.count(), units));
      });
   };

   ProcessSelection(selector, selectionCallback);
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

std::wstring Controller::ResolveCompleteFilePath(const Tree<VizFile>::Node& node)
{
   std::vector<std::wstring> reversePath;
   reversePath.reserve(Tree<VizFile>::Depth(node));
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
      const auto shouldAddSlash = !path.empty() && path.back() != L'\\';
      return path + (shouldAddSlash ? L"\\" : L"") + file;
   });

   assert(completePath.size() > 0);
   return completePath + node->file.extension;
}

void Controller::ShowInFileExplorer(const Tree<VizFile>::Node& node)
{
   CoInitializeEx(NULL, COINIT_MULTITHREADED);
   ON_SCOPE_EXIT noexcept { CoUninitialize(); };

   std::wstring filePath = Controller::ResolveCompleteFilePath(node);

   assert(std::none_of(std::begin(filePath), std::end(filePath),
      [] (const auto character)
   {
      return character == L'/';
   }));

   auto* idList = ILCreateFromPath(filePath.c_str());
   if (idList)
   {
      SHOpenFolderAndSelectItems(idList, 0, 0, 0);
      ILFree(idList);
   }
}
