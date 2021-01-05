#include "controller.h"

#include "Model/squarifiedTreemap.h"
#include "Settings/persistentSettings.h"
#include "Utilities/ignoreUnused.h"
#include "Utilities/operatingSystem.h"
#include "Utilities/scopeExit.h"
#include "View/mainWindow.h"
#include "constants.h"
#include "literals.h"

#include <gsl/gsl_assert>
#include <spdlog/spdlog.h>
#include <stopwatch.h>

#include <algorithm>
#include <utility>

#include <QCursor>

namespace
{
    /**
     * @brief Helper function to be called once scanning completes.
     *
     * @param[in] progress           The final results from the scan.
     */
    void LogScanCompletion(const ScanningProgress& progress)
    {
        const auto& log = spdlog::get(Constants::Logging::DefaultLog);

        log->info(fmt::format(
            "Scanned: {:n} directories and {:n} files, representing {:n} bytes.",
            progress.directoriesScanned.load(), progress.filesScanned.load(),
            progress.bytesProcessed.load()));

        log->flush();
    }

    /**
     * @brief Helper function to log basic disk statistics.
     *
     * @param[in] spaceInfo         Disk information.
     */
    void LogDiskStatistics(const std::filesystem::space_info& spaceInfo)
    {
        const auto& log = spdlog::get(Constants::Logging::DefaultLog);
        log->info(fmt::format("Disk Size:  {:n} bytes.", spaceInfo.capacity));
        log->info(fmt::format("Free Space: {:n} bytes.", spaceInfo.free));
    }
} // namespace

Controller::Controller(ViewFactoryInterface& viewFactory, ModelFactoryInterface& modelFactory)
    : m_viewFactory{ viewFactory },
      m_modelFactory{ modelFactory },
      m_persistentSettings{ Settings::PersistentSettings::DefaultPreferencesFilePath() },
      m_view{ viewFactory.CreateView(*this) }
{
}

void Controller::LaunchUI()
{
    m_view->Show();
}

void Controller::OnScanComplete(
    const Settings::VisualizationParameters& parameters, const ScanningProgress& progress,
    const std::shared_ptr<Tree<VizBlock>>& scanningResults)
{
    LogScanCompletion(progress);
    ReportProgressToStatusBar(progress);

    m_nodeColorMap.clear();

    m_view->AskUserToLimitFileSize(progress.filesScanned.load(), parameters);
    m_view->SetWaitCursor();

    const ScopeExit restoreCursor = [&]() noexcept
    {
        m_view->RestoreDefaultCursor();
    };

    m_model->Parse(scanningResults);
    m_model->UpdateBoundingBoxes();

    SaveScanMetadata(progress);

    m_view->OnScanCompleted();

    try {
        const auto shouldEnable = GetPersistentSettings().ShouldMonitorFileSystem();
        MonitorFileSystem(shouldEnable);
    } catch (const std::exception& exception) {
        m_view->DisplayErrorDialog(exception.what());
    }

    AllowUserInteractionWithModel(true);

    emit FinishedScanning();
}

void Controller::MonitorFileSystem(bool shouldEnable)
{
    if (!HasModelBeenLoaded()) {
        return;
    }

    if (shouldEnable) {
        m_model->StartMonitoringFileSystem();
    } else {
        m_model->StopMonitoringFileSystem();
    }
}

void Controller::ScanDrive(const Settings::VisualizationParameters& parameters)
{
    const auto& root = parameters.rootDirectory;

    if (root.empty() || !ScanningWorker::IsScannable(root)) {
        return;
    }

    AllowUserInteractionWithModel(false);

    m_model = m_modelFactory.CreateModel(std::make_unique<FileSystemMonitor>(), root);
    m_view->OnScanStarted();

    const auto spaceInfo = std::filesystem::space(root);
    LogDiskStatistics(spaceInfo);

    m_occupiedDiskSpace = spaceInfo.capacity - spaceInfo.free;
    Expects(m_occupiedDiskSpace > 0u);

    m_taskbarProgress = m_view->GetTaskbarButton();
    m_taskbarProgress->SetWindow(m_view->GetWindowHandle());

    const auto progressHandler = [&](const ScanningProgress& progress) {
        ReportProgressToStatusBar(progress);
        ReportProgressToTaskbar(*m_taskbarProgress, progress);
    };

    const auto completionHandler =
        [&](const ScanningProgress& progress,
            const std::shared_ptr<Tree<VizBlock>>& scanningResults) mutable {
            OnScanComplete(parameters, progress, scanningResults);

            m_taskbarProgress->HideProgress();
            m_taskbarProgress->ResetProgress();
        };

    spdlog::get(Constants::Logging::DefaultLog)
        ->info(fmt::format("Started a new scan at \"{}\".", m_model->GetRootPath().string()));

    const auto scanningParameters = ScanningParameters{ root, progressHandler, completionHandler };
    m_scanner.StartScanning(scanningParameters);
}

void Controller::StopScanning()
{
    m_scanner.StopProgressReporting();
    m_scanner.StopScanning();
}

bool Controller::IsFileSystemBeingMonitored() const
{
    return m_model->IsFileSystemBeingMonitored();
}

std::optional<FileEvent> Controller::FetchNextFileModification()
{
    return m_model->FetchNextVisualChange();
}

QVector3D Controller::DetermineNodeColor(const Tree<VizBlock>::Node& node) const
{
    const auto nodeColor = m_nodeColorMap.find(reinterpret_cast<std::uintptr_t>(&node));
    if (nodeColor != std::end(m_nodeColorMap)) {
        return nodeColor->second;
    }

    if (IsNodeHighlighted(node)) {
        return Constants::Colors::Highlighted;
    }

    if (m_nodePainter.GetActiveColorScheme() != Constants::ColorScheme::Default) {
        const auto fileColor =
            m_nodePainter.DetermineColorFromExtension(node.GetData().file.extension);

        if (fileColor) {
            return *fileColor;
        }
    }

    if (node->file.type == FileType::Directory) {
        return Constants::Colors::Directory;
    }

    Expects(node->file.type == FileType::Regular);
    return Constants::Colors::File;
}

void Controller::ReportProgressToStatusBar(const ScanningProgress& progress)
{
    Expects(m_occupiedDiskSpace > 0u);

    const auto filesScanned = progress.filesScanned.load();
    const auto sizeInBytes = progress.bytesProcessed.load();

    const auto elapsedTime = progress.GetElapsedSeconds().count();
    const auto hours = elapsedTime / 3600;
    const auto minutes = (elapsedTime / 60) % 60;
    const auto seconds = elapsedTime % 60;

    const auto rootPath = m_model->GetRootPath();
    const auto doesPathRepresentEntireDrive{ rootPath == rootPath.root_path() };

    if (doesPathRepresentEntireDrive) {
        const auto fraction = sizeInBytes / static_cast<double>(m_occupiedDiskSpace);
        const auto message = fmt::format(
            L"Time Elapsed: {:02n}:{:02n}:{:02n}  |  Files Scanned: {:n}  |  {:03.2f}% Complete",
            hours, minutes, seconds, filesScanned, fraction * 100);

        m_view->SetStatusBarMessage(message);
    } else {
        const auto prefix = m_sessionSettings.GetActiveNumericPrefix();
        const auto [size, units] = Utilities::ToPrefixedSize(sizeInBytes, prefix);

        const auto message = fmt::format(
            L"Time Elapsed: {:02n}:{:02n}:{:02n}  |  Files Scanned: {:n}  |  {:03.2f} {} and "
            L"counting...",
            hours, minutes, seconds, filesScanned, size, units);

        m_view->SetStatusBarMessage(message);
    }
}

bool Controller::HasModelBeenLoaded() const
{
    return m_model != nullptr;
}

const Tree<VizBlock>::Node* Controller::GetSelectedNode() const
{
    Expects(m_model);
    return m_model->GetSelectedNode();
}

Tree<VizBlock>& Controller::GetTree()
{
    Expects(m_model);
    return m_model->GetTree();
}

const Tree<VizBlock>& Controller::GetTree() const
{
    Expects(m_model);
    return m_model->GetTree();
}

const std::vector<const Tree<VizBlock>::Node*>& Controller::GetHighlightedNodes() const
{
    Expects(m_model);
    return m_model->GetHighlightedNodes();
}

bool Controller::IsNodeHighlighted(const Tree<VizBlock>::Node& node) const
{
    Expects(m_model);
    const auto& highlightedNodes = m_model->GetHighlightedNodes();

    return std::any_of(
        std::begin(highlightedNodes),
        std::end(highlightedNodes), [target = std::addressof(node)](auto ptr) noexcept {
            return ptr == target;
        });
}

void Controller::SelectNode(
    const Tree<VizBlock>::Node& node,
    const std::function<void(const Tree<VizBlock>::Node&)>& selectorCallback)
{
    Expects(m_model);

    m_model->SelectNode(node);
    selectorCallback(node);
}

void Controller::SelectNodeAndUpdateStatusBar(
    const Tree<VizBlock>::Node& node,
    const std::function<void(const Tree<VizBlock>::Node&)>& selectorCallback)
{
    Expects(node->file.size > 0);

    SelectNode(node, selectorCallback);

    const auto fileSize = node->file.size;
    const auto prefix = m_sessionSettings.GetActiveNumericPrefix();
    const auto [prefixedSize, units] = Utilities::ToPrefixedSize(fileSize, prefix);
    const auto isSmallFile = (units == Utilities::Detail::bytesLabel);

    const auto path = Controller::NodeToFilePath(node).wstring();
    const auto message = isSmallFile ? fmt::format(L"{}  |  {:.0f} {}", path, prefixedSize, units)
                                     : fmt::format(L"{}  |  {:.2f} {}", path, prefixedSize, units);

    m_view->SetStatusBarMessage(message);
}

void Controller::SelectNodeViaRay(
    const Camera& camera, const Ray& ray,
    const std::function<void(const Tree<VizBlock>::Node&)>& deselectionCallback,
    const std::function<void(const Tree<VizBlock>::Node&)>& selectionCallback)
{
    if (!HasModelBeenLoaded() || !IsUserAllowedToInteractWithModel()) {
        return;
    }

    const auto* const selectedNode = m_model->GetSelectedNode();

    if (selectedNode) {
        deselectionCallback(*selectedNode);
        m_model->ClearSelectedNode();
    }

    const auto& parameters = m_sessionSettings.GetVisualizationParameters();
    const auto* node = m_model->FindNearestIntersection(camera, ray, parameters);

    if (node) {
        SelectNodeAndUpdateStatusBar(*node, selectionCallback);
    } else {
        PrintMetadataToStatusBar();
    }
}

void Controller::PrintMetadataToStatusBar()
{
    const auto metadata = m_model->GetTreemapMetadata();
    const auto message = fmt::format(
        L"Scanned {:n} files and {:n} directories.", metadata.FileCount, metadata.DirectoryCount);

    m_view->SetStatusBarMessage(message);
}

void Controller::DisplayHighlightDetails()
{
    const auto& highlightedNodes = m_model->GetHighlightedNodes();

    std::uintmax_t totalBytes = 0;
    for (const auto* const node : highlightedNodes) {
        totalBytes += node->GetData().file.size;
    }

    const auto prefix = m_sessionSettings.GetActiveNumericPrefix();
    const auto [prefixedSize, units] = Utilities::ToPrefixedSize(totalBytes, prefix);
    const auto isSmallFile = (units == Utilities::Detail::bytesLabel);

    const std::wstring nodes = highlightedNodes.size() == 1 ? L" node" : L" nodes";

    if (isSmallFile) {
        m_view->SetStatusBarMessage(fmt::format(
            L"Highlighted {:n} " + nodes + L", presenting {:.0f} {}.", highlightedNodes.size(),
            prefixedSize, units));
    } else {
        m_view->SetStatusBarMessage(fmt::format(
            L"Highlighted {:n} " + nodes + L", presenting {:.2f} {}.", highlightedNodes.size(),
            prefixedSize, units));
    }
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
    Expects(m_model);
    m_model->SetTreemapMetadata(TreemapMetadata{ progress.filesScanned.load(),
                                                 progress.directoriesScanned.load(),
                                                 progress.bytesProcessed.load() });
}

void Controller::ClearSelectedNode()
{
    Expects(m_model);
    m_model->ClearSelectedNode();
}

void Controller::ClearHighlightedNodes(
    const std::function<void(std::vector<const Tree<VizBlock>::Node*>&)>& callback)
{
    Expects(m_model);

    // @note This is a deliberate copy. The nodes have to be cleared from the model before we can
    // ask the UI to update. This is because the UI will call back into the controller to determine
    // if a node is highlighted (so that we can choose the appropriate color).
    auto nodes = m_model->GetHighlightedNodes();
    m_model->ClearHighlightedNodes();

    callback(nodes);
}

template <typename NodeSelectorType>
void Controller::ProcessHighlightedNodes(
    const NodeSelectorType& nodeSelector,
    const std::function<void(std::vector<const Tree<VizBlock>::Node*>&)>& callback)
{
    Expects(m_model);
    nodeSelector();

    auto& nodes = m_model->GetHighlightedNodes();
    callback(nodes);

    DisplayHighlightDetails();
}

void Controller::HighlightAncestors(
    const Tree<VizBlock>::Node& node,
    const std::function<void(std::vector<const Tree<VizBlock>::Node*>&)>& callback)
{
    Expects(m_model);

    const auto selector = [&] { m_model->HighlightAncestors(node); };

    ProcessHighlightedNodes(selector, callback);
}

void Controller::HighlightDescendants(
    const Tree<VizBlock>::Node& node,
    const std::function<void(std::vector<const Tree<VizBlock>::Node*>&)>& callback)
{
    Expects(m_model);

    const auto selector = [&] {
        m_model->HighlightDescendants(node, m_sessionSettings.GetVisualizationParameters());
    };

    ProcessHighlightedNodes(selector, callback);
}

void Controller::HighlightAllMatchingExtensions(
    const std::wstring& extension,
    const std::function<void(std::vector<const Tree<VizBlock>::Node*>&)>& callback)
{
    Expects(m_model);

    const auto& parameters = m_sessionSettings.GetVisualizationParameters();
    const auto selector = [&] { m_model->HighlightMatchingFileExtensions(extension, parameters); };

    ProcessHighlightedNodes(selector, callback);
}

void Controller::SearchTreeMap(
    const std::wstring& searchQuery,
    const std::function<void(std::vector<const Tree<VizBlock>::Node*>&)>& deselectionCallback,
    const std::function<void(std::vector<const Tree<VizBlock>::Node*>&)>& selectionCallback,
    bool shouldSearchFiles, bool shouldSearchDirectories)
{
    if (searchQuery.empty() || !HasModelBeenLoaded() ||
        (!shouldSearchFiles && !shouldSearchDirectories)) {
        return;
    }

    ClearHighlightedNodes(deselectionCallback);

    const auto selector = [&] {
        const auto stopwatch = Stopwatch<std::chrono::milliseconds>([&]() noexcept {
            m_model->HighlightMatchingFileNames(
                searchQuery, m_sessionSettings.GetVisualizationParameters(), shouldSearchFiles,
                shouldSearchDirectories);
        });

        spdlog::get(Constants::Logging::DefaultLog)
            ->info(fmt::format(
                "Search Completed in: {:n} {}.", stopwatch.GetElapsedTime().count(),
                stopwatch.GetUnitsAsString()));
    };

    ProcessHighlightedNodes(selector, selectionCallback);
}

std::filesystem::path Controller::NodeToFilePath(const Tree<VizBlock>::Node& node)
{
    std::vector<std::reference_wrapper<const std::wstring>> reversePath;
    reversePath.reserve(Tree<VizBlock>::Depth(node));
    reversePath.emplace_back(node->file.name);

    const auto* currentNode = &node;
    while (currentNode->GetParent()) {
        currentNode = currentNode->GetParent();
        reversePath.emplace_back(currentNode->GetData().file.name);
    }

    const auto completePath = std::accumulate(
        std::rbegin(reversePath), std::rend(reversePath), std::wstring{},
        [](const std::wstring& path, const std::wstring& file) {
            constexpr auto slash = L'/';

            if (!path.empty() && path.back() != slash) {
                return path + slash + file;
            }

            return path + file;
        });

    Expects(completePath.empty() == false);

    auto finalPath = std::filesystem::path{ completePath + node->file.extension };
    finalPath.make_preferred();

    return finalPath;
}

Settings::PersistentSettings& Controller::GetPersistentSettings()
{
    return m_persistentSettings;
}

const Settings::PersistentSettings& Controller::GetPersistentSettings() const
{
    return m_persistentSettings;
}

Settings::NodePainter& Controller::GetNodePainter()
{
    return m_nodePainter;
}

const Settings::NodePainter& Controller::GetNodePainter() const
{
    return m_nodePainter;
}

Settings::SessionSettings& Controller::GetSessionSettings()
{
    return m_sessionSettings;
}

const Settings::SessionSettings& Controller::GetSessionSettings() const
{
    return m_sessionSettings;
}

std::filesystem::path Controller::GetRootPath() const
{
    return m_model->GetRootPath();
}

void Controller::RegisterNodeColor(const Tree<VizBlock>::Node& node, const QVector3D& color)
{
    // @note This tracking only works if we assume that nodes are never copied; this may prove to be
    // a bad assumption. Consider using an ID on the node instead.
    m_nodeColorMap.insert_or_assign(reinterpret_cast<std::uintptr_t>(&node), color);
}

template <typename ButtonType>
void Controller::ReportProgressToTaskbar(ButtonType& button, const ScanningProgress& progress)
{
    IgnoreUnused(button, progress);

#if defined(Q_OS_WIN)
    const auto sizeInBytes = progress.bytesProcessed.load();

    const auto rootPath = m_model->GetRootPath();
    const auto doesPathRepresentEntireDrive{ rootPath == rootPath.root_path() };

    button.SetVisible(true);

    if (doesPathRepresentEntireDrive) {
        const auto progressValue = sizeInBytes / static_cast<double>(m_occupiedDiskSpace);
        button.SetValue(static_cast<int>(100.0 * progressValue));
    } else {
        button.SetMinimum(0);
        button.SetMaximum(0);
    }
#endif // Q_OS_WIN
}
