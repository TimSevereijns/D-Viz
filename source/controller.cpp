#include "controller.h"

#include "constants.h"
#include "Utilities/ignoreUnused.hpp"
#include "Utilities/operatingSystemSpecific.hpp"
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

namespace
{
   constexpr const wchar_t BYTES_READOUT_STRING[] = L" bytes";

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

   /**
    * @brief Converts bytes to binary prefix size and notation.
    *
    * @param[in] sizeInBytes
    *
    * @returns A pair containing the numeric file size, and the associated units.
    */
   std::pair<double, std::wstring> ConvertToBinaryPrefix(double sizeInBytes)
   {
      if (sizeInBytes < Constants::FileSize::Binary::ONE_KIBIBYTE)
      {
         return std::make_pair<double, std::wstring>(std::move(sizeInBytes), BYTES_READOUT_STRING);
      }

      if (sizeInBytes < Constants::FileSize::Binary::ONE_MEBIBYTE)
      {
         return std::make_pair<double, std::wstring>(
            sizeInBytes / Constants::FileSize::Binary::ONE_KIBIBYTE, L" KiB");
      }

      if (sizeInBytes < Constants::FileSize::Binary::ONE_GIBIBYTE)
      {
         return std::make_pair<double, std::wstring>(
            sizeInBytes / Constants::FileSize::Binary::ONE_MEBIBYTE, L" MiB");
      }

      if (sizeInBytes < Constants::FileSize::Binary::ONE_TEBIBYTE)
      {
         return std::make_pair<double, std::wstring>(
            sizeInBytes / Constants::FileSize::Binary::ONE_GIBIBYTE, L" GiB");
      }

      return std::make_pair<double, std::wstring>(
         sizeInBytes / Constants::FileSize::Binary::ONE_TEBIBYTE, L" TiB");
   }

   /**
    * @brief Converts bytes to decimal prefix size and notation.
    *
    * @param[in] sizeInBytes
    *
    * @returns A pair containing the numeric file size, and the associated units.
    */
   std::pair<double, std::wstring> ConvertToDecimalPrefix(double sizeInBytes)
   {
      if (sizeInBytes < Constants::FileSize::Decimal::ONE_KILOBYTE)
      {
         return std::make_pair<double, std::wstring>(std::move(sizeInBytes), BYTES_READOUT_STRING);
      }

      if (sizeInBytes < Constants::FileSize::Decimal::ONE_MEGABYTE)
      {
         return std::make_pair<double, std::wstring>(
            sizeInBytes / Constants::FileSize::Decimal::ONE_KILOBYTE, L" KB");
      }

      if (sizeInBytes < Constants::FileSize::Decimal::ONE_GIGABYTE)
      {
         return std::make_pair<double, std::wstring>(
            sizeInBytes / Constants::FileSize::Decimal::ONE_MEGABYTE, L" MB");
      }

      if (sizeInBytes < Constants::FileSize::Decimal::ONE_TERABYTE)
      {
         return std::make_pair<double, std::wstring>(
            sizeInBytes / Constants::FileSize::Decimal::ONE_GIGABYTE, L" GB");
      }

      return std::make_pair<double, std::wstring>(
         sizeInBytes / Constants::FileSize::Decimal::ONE_TERABYTE, L" TB");
   }
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
      m_highlightedNodes.clear();
      m_selectedNode = nullptr;

      m_treeMap = std::make_unique<SquarifiedTreeMap>(m_visualizationParameters);
      m_mainWindow->ScanDrive(m_visualizationParameters);
   }
}

const Tree<VizFile>::Node* Controller::GetSelectedNode() const
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

   const auto [size, units] = Controller::ConvertFileSizeToAppropriateUnits(fileSize);
   const auto isInBytes = (units == BYTES_READOUT_STRING);

   std::wstringstream message;
   message.imbue(std::locale{ "" });
   message.precision(isInBytes ? 0 : 2);
   message << std::fixed << Controller::ResolveCompleteFilePath(*node) << L"  |  " << size << units;

   assert(message.str().size() > 0);
   m_mainWindow->SetStatusBarMessage(message.str());

   selectorCallback(node);
}

void Controller::SelectNodeViaRay(
   const Camera& camera,
   const Qt3DRender::RayCasting::QRay3D& ray,
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
      PrintMetadataToStatusBar();
   }
}

void Controller::PrintMetadataToStatusBar()
{
   std::wstringstream message;
   message.imbue(std::locale{ "" });
   message
      << std::fixed
      << L"Scanned "
      << m_filesInCurrentVisualization
      << L" files and "
      << m_directoriesInCurrentVisualization
      << L" directories.";

   m_mainWindow->SetStatusBarMessage(message.str());
}

void Controller::PrintSelectionDetailsToStatusBar()
{
   std::uintmax_t selectionSizeInBytes{ 0 };
   for (const auto* const node : m_highlightedNodes)
   {
      selectionSizeInBytes += node->GetData().file.size;
   }

   const auto [size, units] = Controller::ConvertFileSizeToAppropriateUnits(selectionSizeInBytes);
   const auto isInBytes = (units == BYTES_READOUT_STRING);

   std::wstringstream message;
   message.imbue(std::locale{ "" });
   message.precision(isInBytes ? 0 : 2);
   message
      << L"Highlighted " << m_highlightedNodes.size()
      << (m_highlightedNodes.size() == 1 ? L" node" : L" nodes")
      << L", representing " << size << units;

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

void Controller::SaveScanResults(const ScanningProgress& progress)
{
   m_filesInCurrentVisualization = progress.filesScanned.load();
   m_directoriesInCurrentVisualization = progress.directoriesScanned.load();
   m_totalBytesInCurrentVisualization = progress.bytesProcessed.load();
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
      //WideStackString<540>::allocator_type::arena_type stringArena{ };
      //WideStackString<540> fullName{ std::move(stringArena) };
      //fullName.resize(260); ///< Resize to prevent reallocation with later append operations.

      std::wstring fullName;
      fullName.resize(260);

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
   std::uintmax_t sizeInBytes)
{
   switch (ActivePrefix)
   {
      case Constants::FileSize::Prefix::BINARY:
         return ConvertToBinaryPrefix(sizeInBytes);
      case Constants::FileSize::Prefix::DECIMAL:
         return ConvertToDecimalPrefix(sizeInBytes);
   }

   assert(false);
   return std::make_pair<double, std::wstring>( 0, L"Congrats, you've found a bug!" );
}

std::wstring Controller::ResolveCompleteFilePath(const Tree<VizFile>::Node& node)
{
   std::vector<std::reference_wrapper<const std::wstring>> reversePath;
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
      if (!path.empty() && path.back() != OperatingSystemSpecific::PREFERRED_SLASH)
      {
         return path + OperatingSystemSpecific::PREFERRED_SLASH + file;
      }

      return path + file;
   });

   assert(completePath.size() > 0);
   return completePath + node->file.extension;
}
