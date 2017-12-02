#include "controller.h"

#include "constants.h"
#include "literals.h"

#include "Settings/settingsManager.h"
#include "Utilities/ignoreUnused.hpp"
#include "Utilities/operatingSystemSpecific.hpp"
#include "Utilities/scopeExit.hpp"
#include "Visualizations/squarifiedTreemap.h"
#include "Windows/mainWindow.h"

#include <boost/algorithm/string/case_conv.hpp>
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

   /**
    * @brief Converts bytes to binary prefix size and notation.
    *
    * @param[in] sizeInBytes
    *
    * @returns A pair containing the numeric file size, and the associated units.
    */
   std::pair<double, std::wstring> ConvertToBinaryPrefix(double sizeInBytes)
   {
      using namespace Literals::Numeric::Binary;

      if (sizeInBytes < 1_KiB)
      {
         return std::make_pair<double, std::wstring>(std::move(sizeInBytes), BYTES_READOUT_STRING);
      }

      if (sizeInBytes < 1_MiB)
      {
         return std::make_pair<double, std::wstring>(sizeInBytes / 1_KiB, L" KiB");
      }

      if (sizeInBytes < 1_GiB)
      {
         return std::make_pair<double, std::wstring>(sizeInBytes / 1_MiB, L" MiB");
      }

      if (sizeInBytes < 1_TiB)
      {
         return std::make_pair<double, std::wstring>(sizeInBytes / 1_GiB, L" GiB");
      }

      return std::make_pair<double, std::wstring>(sizeInBytes / 1_TiB, L" TiB");
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
      using namespace Literals::Numeric::Decimal;

      if (sizeInBytes < 1_KB)
      {
         return std::make_pair<double, std::wstring>(std::move(sizeInBytes), BYTES_READOUT_STRING);
      }

      if (sizeInBytes < 1_MB)
      {
         return std::make_pair<double, std::wstring>(sizeInBytes / 1_KB, L" KB");
      }

      if (sizeInBytes < 1_GB)
      {
         return std::make_pair<double, std::wstring>(sizeInBytes / 1_MB, L" MB");
      }

      if (sizeInBytes < 1_TB)
      {
         return std::make_pair<double, std::wstring>(sizeInBytes / 1_GB, L" GB");
      }

      return std::make_pair<double, std::wstring>(sizeInBytes / 1_TB, L" TB");
   }
}

bool Controller::HasVisualizationBeenLoaded() const
{
   return m_treeMap != nullptr;
}

void Controller::ResetVisualization()
{
   m_highlightedNodes.clear();

   m_selectedNode = nullptr;
   m_treeMap = nullptr;
}

const Tree<VizFile>::Node* Controller::GetSelectedNode() const
{
   return m_selectedNode;
}

Tree<VizFile>& Controller::GetTree()
{
   assert(m_treeMap);

   return m_treeMap->GetTree();
}

const Tree<VizFile>& Controller::GetTree() const
{
   assert(m_treeMap);

   return m_treeMap->GetTree();
}

const std::vector<const Tree<VizFile>::Node*>& Controller::GetHighlightedNodes() const
{
   return m_highlightedNodes;
}

bool Controller::IsNodeHighlighted(const Tree<VizFile>::Node& node) const
{
   return std::any_of(std::begin(m_highlightedNodes), std::end(m_highlightedNodes),
      [target = std::addressof(node)] (auto ptr) noexcept
   {
      return ptr == target;
   });
}

void Controller::SetView(MainWindow* window)
{
   assert(m_mainWindow == nullptr);
   assert(window);

   m_mainWindow = window;
}

void Controller::ParseResults(const std::shared_ptr<Tree<VizFile>>& results)
{
   assert(!m_treeMap);

   m_treeMap = std::make_unique<SquarifiedTreeMap>();
   m_treeMap->Parse(results);
}

void Controller::UpdateBoundingBoxes()
{
   assert(m_treeMap);

   m_treeMap->UpdateBoundingBoxes();
}

void Controller::SelectNode(
   const Tree<VizFile>::Node& node,
   const std::function<void (const Tree<VizFile>::Node&)>& selectorCallback)
{
   m_selectedNode = &node;

   selectorCallback(node);
}

void Controller::SelectNodeAndUpdateStatusBar(
   const Tree<VizFile>::Node& node,
   const std::function<void (const Tree<VizFile>::Node&)>& selectorCallback)
{
   SelectNode(node, selectorCallback);

   const auto fileSize = node->file.size;
   assert(fileSize > 0);

   const auto prefix = m_mainWindow->GetSettingsManager().GetActiveNumericPrefix();
   const auto [prefixedSize, units] = ConvertFileSizeToNumericPrefix(fileSize, prefix);
   const auto isInBytes = (units == BYTES_READOUT_STRING);

   std::wstringstream message;
   message.imbue(std::locale{ "" });
   message.precision(isInBytes ? 0 : 2);
   message
      << std::fixed
      << Controller::ResolveCompleteFilePath(node)
      << L"  |  "
      << prefixedSize
      << units;

   m_mainWindow->SetStatusBarMessage(message.str());
}

void Controller::SelectNodeViaRay(
   const Camera& camera,
   const Qt3DRender::RayCasting::QRay3D& ray,
   const std::function<void (const Tree<VizFile>::Node&)>& deselectionCallback,
   const std::function<void (const Tree<VizFile>::Node&)>& selectionCallback)
{
   if (!HasVisualizationBeenLoaded() || !IsUserAllowedToInteractWithModel())
   {
      return;
   }

   deselectionCallback(*m_selectedNode);
   m_selectedNode = nullptr;

   const auto& parameters = m_mainWindow->GetSettingsManager().GetVisualizationParameters();
   const auto* node = m_treeMap->FindNearestIntersection(camera, ray, parameters);
   if (node)
   {
      SelectNodeAndUpdateStatusBar(*node, selectionCallback);
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
   std::uintmax_t totalBytes{ 0 };
   for (const auto* const node : m_highlightedNodes)
   {
      totalBytes += node->GetData().file.size;
   }

   const auto prefix = m_mainWindow->GetSettingsManager().GetActiveNumericPrefix();
   const auto [prefixedSize, units] = ConvertFileSizeToNumericPrefix(totalBytes, prefix);
   const auto isInBytes = (units == BYTES_READOUT_STRING);

   std::wstringstream message;
   message.imbue(std::locale{ "" });
   message.precision(isInBytes ? 0 : 2);
   message
      << std::fixed
      << L"Highlighted " << m_highlightedNodes.size()
      << (m_highlightedNodes.size() == 1 ? L" node" : L" nodes")
      << L", representing "
      << prefixedSize
      << units;

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
   const std::function<void (std::vector<const Tree<VizFile>::Node*>&)>& callback)
{
   if (m_highlightedNodes.size() == 0)
   {
      return;
   }

   const auto victim = std::find_if(
      std::begin(m_highlightedNodes),
      std::end(m_highlightedNodes),
      [target = m_selectedNode] (auto ptr) noexcept
   {
      return ptr == target;
   });

   if (victim != std::end(m_highlightedNodes))
   {
      m_highlightedNodes.erase(victim);
   }

   callback(m_highlightedNodes);
   m_highlightedNodes.clear();
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
      auto* currentNode = node.GetParent();
      while (currentNode)
      {
         m_highlightedNodes.emplace_back(currentNode);
         currentNode = currentNode->GetParent();
      }
   };

   ProcessSelection(selector, callback);
}

void Controller::HighlightDescendants(
   const Tree<VizFile>::Node& node,
   const std::function<void (std::vector<const Tree<VizFile>::Node*>&)>& callback)
{
   const auto& parameters = m_mainWindow->GetSettingsManager().GetVisualizationParameters();

   const auto selector = [&]
   {
      std::for_each(
         Tree<VizFile>::LeafIterator{ &node },
         Tree<VizFile>::LeafIterator{ },
         [&] (const auto& node)
      {
         if ((parameters.onlyShowDirectories && node->file.type != FileType::DIRECTORY)
            || node->file.size < parameters.minimumFileSize)
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
   const auto& parameters = m_mainWindow->GetSettingsManager().GetVisualizationParameters();

   const auto selector = [&]
   {
      std::for_each(
         Tree<VizFile>::LeafIterator{ GetTree().GetRoot() },
         Tree<VizFile>::LeafIterator{ },
         [&] (const auto& node)
      {
         if ((parameters.onlyShowDirectories && node->file.type != FileType::DIRECTORY)
            || node->file.size < parameters.minimumFileSize
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

   ClearHighlightedNodes(deselectionCallback);

   const auto& parameters = m_mainWindow->GetSettingsManager().GetVisualizationParameters();

   const auto selector = [&]
   {
      std::wstring fileAndExtension;
      fileAndExtension.resize(260); ///< Resize to prevent reallocation with append operations.

      const auto lowercaseQuery = boost::algorithm::to_lower_copy(searchQuery);

      Stopwatch<std::chrono::milliseconds>([&] () noexcept
      {
         std::for_each(
            Tree<VizFile>::PostOrderIterator{ GetTree().GetRoot() },
            Tree<VizFile>::PostOrderIterator{ },
            [&] (const auto& node)
         {
            const auto& file = node->file;

            if (file.size < parameters.minimumFileSize
               || (!shouldSearchDirectories && file.type == FileType::DIRECTORY)
               || (!shouldSearchFiles && file.type == FileType::REGULAR))
            {
               return;
            }

            fileAndExtension = file.name;
            fileAndExtension.append(file.extension);

            boost::algorithm::to_lower(fileAndExtension);

            // @note We're converting everything to lowercase beforehand
            // (instead of using `boost::icontains(...)`), since doing so is significantly faster.
            if (!boost::contains(fileAndExtension, lowercaseQuery))
            {
               return;
            }

            m_highlightedNodes.emplace_back(&node);
         });
      }, [] (const auto& elapsed, const auto& units) noexcept
      {
         spdlog::get(Constants::Logging::DEFAULT_LOG)->info(
            fmt::format("Search Completed in: {} {}", elapsed.count(), units));
      });
   };

   ProcessSelection(selector, selectionCallback);
}

std::pair<double, std::wstring> Controller::ConvertFileSizeToNumericPrefix(
   std::uintmax_t sizeInBytes,
   Constants::FileSize::Prefix prefix)
{
   switch (prefix)
   {
      case Constants::FileSize::Prefix::BINARY:
      {
         return ConvertToBinaryPrefix(sizeInBytes);
      }
      case Constants::FileSize::Prefix::DECIMAL:
      {
         return ConvertToDecimalPrefix(sizeInBytes);
      }
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
