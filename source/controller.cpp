#include "controller.h"

#include "constants.h"
#include "literals.h"

#include "Settings/settingsManager.h"
#include "Utilities/ignoreUnused.hpp"
#include "Utilities/operatingSystemSpecific.hpp"
#include "Utilities/scopeExit.hpp"
#include "Visualizations/squarifiedTreemap.h"
#include "Windows/mainWindow.h"

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
         return std::make_pair(sizeInBytes, BYTES_READOUT_STRING);
      }

      if (sizeInBytes < 1_MiB)
      {
         return std::make_pair(sizeInBytes / 1_KiB, L" KiB");
      }

      if (sizeInBytes < 1_GiB)
      {
         return std::make_pair(sizeInBytes / 1_MiB, L" MiB");
      }

      if (sizeInBytes < 1_TiB)
      {
         return std::make_pair(sizeInBytes / 1_GiB, L" GiB");
      }

      return std::make_pair(sizeInBytes / 1_TiB, L" TiB");
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
         return std::make_pair(sizeInBytes, BYTES_READOUT_STRING);
      }

      if (sizeInBytes < 1_MB)
      {
         return std::make_pair(sizeInBytes / 1_KB, L" KB");
      }

      if (sizeInBytes < 1_GB)
      {
         return std::make_pair(sizeInBytes / 1_MB, L" MB");
      }

      if (sizeInBytes < 1_TB)
      {
         return std::make_pair(sizeInBytes / 1_GB, L" GB");
      }

      return std::make_pair(sizeInBytes / 1_TB, L" TB");
   }

   /**
    * @returns The full path to the JSON file that contains the color mapping.
    */
   auto GetColorJsonPath()
   {
      return std::experimental::filesystem::current_path().append(L"colors.json");
   }

   /**
    * @returns The full path to the JSON file that contains the user preferences.
    */
   auto GetPreferencesJsonPath()
   {
      return std::experimental::filesystem::current_path().append(L"preferences.json");
   }

   /**
    * @brief Helper function to be called once scanning completes.
    *
    * @param[in] progress           The final results from the scan.
    */
   void LogScanCompletion(const ScanningProgress& progress)
   {
      const auto& log = spdlog::get(Constants::Logging::DEFAULT_LOG);

      log->info(
         fmt::format("Scanned: {} directories and {} files, representing {} bytes",
         progress.directoriesScanned.load(),
         progress.filesScanned.load(),
         progress.bytesProcessed.load()));

      log->flush();
   }
}

Controller::Controller() :
   m_settingsManager{ GetColorJsonPath(), GetPreferencesJsonPath() },
   m_view{ std::make_unique<MainWindow>(*this) }
{
}

Controller::~Controller()
{
   // @note Despite the fact that this DTOR doesn't do anything, it needs to be declared here,
   // so that its definition appears after the complete definition of the MainWindow class.
   // Moving the MainWindow header include into the Controller's header isn't possible, since that
   // would create a cyclic dependency.
}

void Controller::LaunchUI()
{
   m_view->show();
}

void Controller::ScanDrive(Settings::VisualizationParameters& parameters)
{
   m_rootPath = parameters.rootDirectory;

   m_view->OnScanStarted();

   m_occupiedDiskSpace = OperatingSystemSpecific::GetUsedDiskSpace(m_rootPath);
   assert(m_occupiedDiskSpace > 0);

   const auto progressHandler = [&] (const ScanningProgress& progress)
   {
      ComputeProgress(progress);
   };

   const auto completionHandler = [&, parameters] (
      const ScanningProgress& progress,
      const std::shared_ptr<Tree<VizBlock>>& scanningResults) mutable
   {
      ComputeProgress(progress);
      LogScanCompletion(progress);

      m_view->AskUserToLimitFileSize(progress.filesScanned.load(), parameters);

      m_view->SetWaitCursor();
      ON_SCOPE_EXIT{ m_view->RestoreDefaultCursor(); };

      ParseResults(scanningResults);
      UpdateBoundingBoxes();
      SaveScanMetadata(progress);

      m_view->OnScanCompleted();

      AllowUserInteractionWithModel(true);

      //m_model->StartFileSystemMonitor();
   };

   ResetVisualization();

   AllowUserInteractionWithModel(false);

   DriveScanningParameters scanningParameters
   {
      parameters.rootDirectory,
      progressHandler,
      completionHandler
   };

   const auto& log = spdlog::get(Constants::Logging::DEFAULT_LOG);
   log->info(fmt::format("Started a new scan at: \"{}\"", m_rootPath.string()));

   m_scanner.StartScanning(std::move(scanningParameters));
}

void Controller::ComputeProgress(const ScanningProgress& progress)
{
   assert(m_occupiedDiskSpace > 0);

   const auto filesScanned = progress.filesScanned.load();
   const auto sizeInBytes = progress.bytesProcessed.load();

   const auto doesPathRepresentEntireDrive{ m_rootPath.string() == m_rootPath.root_path() };
   if (doesPathRepresentEntireDrive)
   {
      const auto percentage =
         100 * (static_cast<double>(sizeInBytes) / static_cast<double>(m_occupiedDiskSpace));

      const auto message = fmt::format(L"Files Scanned: {}  |  {:03.2f}% Complete",
         Utilities::StringifyWithDigitSeparators(filesScanned),
         percentage);

      m_view->SetStatusBarMessage(message.c_str());
   }
   else
   {
      const auto prefix = m_settingsManager.GetActiveNumericPrefix();
      const auto [size, units] = Controller::ConvertFileSizeToNumericPrefix(sizeInBytes, prefix);

      const auto message = fmt::format(L"Files Scanned: {}  |  {:03.2f} {} and counting...",
         Utilities::StringifyWithDigitSeparators(filesScanned), size, units);

      m_view->SetStatusBarMessage(message.c_str());
   }
}

bool Controller::HasVisualizationBeenLoaded() const
{
   return m_model != nullptr;
}

void Controller::ResetVisualization()
{
   if (!m_model)
   {
      return;
   }

   m_model->ClearHighlightedNodes();
   m_model->ClearSelectedNode();
   m_model = nullptr;
}

const Tree<VizBlock>::Node* Controller::GetSelectedNode() const
{
   return m_model->GetSelectedNode();
}

Tree<VizBlock>& Controller::GetTree()
{
   assert(m_model);

   return m_model->GetTree();
}

const Tree<VizBlock>& Controller::GetTree() const
{
   assert(m_model);

   return m_model->GetTree();
}

const std::vector<const Tree<VizBlock>::Node*>& Controller::GetHighlightedNodes() const
{
   return m_model->GetHighlightedNodes();
}

bool Controller::IsNodeHighlighted(const Tree<VizBlock>::Node& node) const
{
   const auto& highlightedNodes = m_model->GetHighlightedNodes();

   return std::any_of(std::begin(highlightedNodes), std::end(highlightedNodes),
      [target = std::addressof(node)] (auto ptr) noexcept
   {
      return ptr == target;
   });
}

void Controller::ParseResults(const std::shared_ptr<Tree<VizBlock>>& results)
{
   assert(!m_model);

   m_model = std::make_unique<SquarifiedTreeMap>();
   m_model->Parse(results);
}

void Controller::UpdateBoundingBoxes()
{
   assert(m_model);

   m_model->UpdateBoundingBoxes();
}

void Controller::SelectNode(
   const Tree<VizBlock>::Node& node,
   const std::function<void (const Tree<VizBlock>::Node&)>& selectorCallback)
{
   m_model->SelectNode(node);

   selectorCallback(node);
}

void Controller::SelectNodeAndUpdateStatusBar(
   const Tree<VizBlock>::Node& node,
   const std::function<void (const Tree<VizBlock>::Node&)>& selectorCallback)
{
   SelectNode(node, selectorCallback);

   const auto fileSize = node->file.size;
   assert(fileSize > 0);

   const auto prefix = m_settingsManager.GetActiveNumericPrefix();
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

   m_view->SetStatusBarMessage(message.str());
}

void Controller::SelectNodeViaRay(
   const Camera& camera,
   const Qt3DRender::RayCasting::QRay3D& ray,
   const std::function<void (const Tree<VizBlock>::Node&)>& deselectionCallback,
   const std::function<void (const Tree<VizBlock>::Node&)>& selectionCallback)
{
   if (!HasVisualizationBeenLoaded() || !IsUserAllowedToInteractWithModel())
   {
      return;
   }

   const auto* const selectedNode = m_model->GetSelectedNode();
   if (selectedNode)
   {
      deselectionCallback(*selectedNode);
      m_model->ClearSelectedNode();
   }

   const auto& parameters = m_settingsManager.GetVisualizationParameters();
   const auto* node = m_model->FindNearestIntersection(camera, ray, parameters);
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
   const auto metadata = m_model->GetTreemapMetadata();

   std::wstringstream message;
   message.imbue(std::locale{ "" });
   message
      << std::fixed
      << L"Scanned "
      << metadata.FileCount
      << L" files and "
      << metadata.DirectoryCount
      << L" directories.";

   m_view->SetStatusBarMessage(message.str());
}

void Controller::DisplaySelectionDetails()
{
   const auto& highlightedNodes = m_model->GetHighlightedNodes();

   std::uintmax_t totalBytes{ 0 };
   for (const auto* const node : highlightedNodes)
   {
      totalBytes += node->GetData().file.size;
   }

   const auto prefix = m_settingsManager.GetActiveNumericPrefix();
   const auto [prefixedSize, units] = ConvertFileSizeToNumericPrefix(totalBytes, prefix);
   const auto isInBytes = (units == BYTES_READOUT_STRING);

   std::wstringstream message;
   message.imbue(std::locale{ "" });
   message.precision(isInBytes ? 0 : 2);
   message
      << std::fixed
      << L"Highlighted " << highlightedNodes.size()
      << (highlightedNodes.size() == 1 ? L" node" : L" nodes")
      << L", representing "
      << prefixedSize
      << units;

   m_view->SetStatusBarMessage(message.str());
}

void Controller::AllowUserInteractionWithModel(bool allowInteraction)
{
   m_allowInteractionWithModel = allowInteraction;
}

bool Controller::IsUserAllowedToInteractWithModel() const
{
   return m_allowInteractionWithModel;
}

void Controller::SaveScanMetadata(const ScanningProgress& progress)
{
   TreemapMetadata data
   {
      progress.filesScanned.load(),
      progress.directoriesScanned.load(),
      progress.bytesProcessed.load()
   };

   m_model->SetTreemapMetadata(std::move(data));
}

void Controller::ClearSelectedNode()
{
   m_model->ClearSelectedNode();
}

void Controller::ClearHighlightedNodes(
   const std::function<void (std::vector<const Tree<VizBlock>::Node*>&)>& callback)
{
   auto& nodes = m_model->GetHighlightedNodes();
   callback(nodes);

   m_model->ClearHighlightedNodes();
}

template<typename NodeSelectorType>
void Controller::ProcessSelection(
   const NodeSelectorType& nodeSelector,
   const std::function<void (std::vector<const Tree<VizBlock>::Node*>&)>& callback)
{
   nodeSelector();

   auto& nodes = m_model->GetHighlightedNodes();
   callback(nodes);

   DisplaySelectionDetails();
}

void Controller::HighlightAncestors(
   const Tree<VizBlock>::Node& node,
   const std::function<void (std::vector<const Tree<VizBlock>::Node*>&)>& callback)
{
   const auto selector = [&]
   {
      m_model->HighlightAncestors(node);
   };

   ProcessSelection(selector, callback);
}

void Controller::HighlightDescendants(
   const Tree<VizBlock>::Node& node,
   const std::function<void (std::vector<const Tree<VizBlock>::Node*>&)>& callback)
{
   const auto selector = [&]
   {
      m_model->HighlightDescendants(node, m_settingsManager.GetVisualizationParameters());
   };

   ProcessSelection(selector, callback);
}

void Controller::HighlightAllMatchingExtensions(
   const Tree<VizBlock>::Node& sampleNode,
   const std::function<void (std::vector<const Tree<VizBlock>::Node*>&)>& callback)
{
   const auto& parameters = m_settingsManager.GetVisualizationParameters();

   const auto selector = [&]
   {
      m_model->HighlightMatchingFileExtension(sampleNode, parameters);
   };

   ProcessSelection(selector, callback);
}

void Controller::SearchTreeMap(
   const std::wstring& searchQuery,
   const std::function<void (std::vector<const Tree<VizBlock>::Node*>&)>& deselectionCallback,
   const std::function<void (std::vector<const Tree<VizBlock>::Node*>&)>& selectionCallback,
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

   const auto selector = [&]
   {
      Stopwatch<std::chrono::milliseconds>([&] () noexcept
      {
         m_model->HighlightMatchingFileName(
            searchQuery,
            m_settingsManager.GetVisualizationParameters(),
            shouldSearchFiles,
            shouldSearchDirectories);
      },
      [] (const auto& elapsed, const auto& units) noexcept
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

std::wstring Controller::ResolveCompleteFilePath(const Tree<VizBlock>::Node& node)
{
   std::vector<std::reference_wrapper<const std::wstring>> reversePath;
   reversePath.reserve(Tree<VizBlock>::Depth(node));
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

Settings::Manager& Controller::GetSettingsManager()
{
   return m_settingsManager;
}

const Settings::Manager& Controller::GetSettingsManager() const
{
   return m_settingsManager;
}

std::experimental::filesystem::path Controller::GetRootPath() const
{
   return m_rootPath;
}
