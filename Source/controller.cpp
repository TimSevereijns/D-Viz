#include "controller.h"

#include "Model/squarifiedTreemap.h"
#include "Settings/persistentSettings.h"
#include "Utilities/ignoreUnused.h"
#include "Utilities/operatingSystem.h"
#include "Utilities/scopeExit.h"
#include "View/mainWindow.h"
#include "constants.h"
#include "literals.h"

#include <gsl/assert>
#include <spdlog/spdlog.h>
#include <stopwatch.h>

#include <algorithm>
#include <utility>

#include <QCursor>

namespace
{
    void LogScanCompletion(const ScanningProgress& progress)
    {
        const auto& log = spdlog::get(Constants::Logging::DefaultLog);

        log->info(
            "Scanned {:n} directories and {:n} files, representing {:n} bytes.",
            progress.directoriesScanned.load(), progress.filesScanned.load(),
            progress.bytesProcessed.load());

        log->flush();
    }

    std::uintmax_t ComputeOccupiedDiskSpace(const std::filesystem::path& path)
    {
        const auto spaceInfo = std::filesystem::space(path);

        const auto& log = spdlog::get(Constants::Logging::DefaultLog);
        log->info("Disk Size:  {:n} bytes.", spaceInfo.capacity);
        log->info("Free Space: {:n} bytes.", spaceInfo.free);

        return spaceInfo.capacity - spaceInfo.free;
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
    const ScanningProgress& progress, const std::shared_ptr<Tree<VizBlock>>& scanningResults)
{
    LogScanCompletion(progress);
    ReportProgressToStatusBar(progress);

    m_nodeColorMap.clear();

    m_view->AskUserToLimitFileSize(progress.filesScanned.load());
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

    m_taskbarProgress->HideProgress();
    m_taskbarProgress->ResetProgress();

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

void Controller::ScanDrive(const Settings::VisualizationOptions& options)
{
    const auto& root = options.rootDirectory;

    if (root.empty() || !ScanningWorker::IsScannable(root)) {
        return;
    }

    AllowUserInteractionWithModel(false);

    m_model = m_modelFactory.CreateModel(std::make_unique<FileSystemMonitor>(), root);
    m_view->OnScanStarted();

    m_occupiedDiskSpace = ComputeOccupiedDiskSpace(root);

    m_taskbarProgress = m_view->GetTaskbarButton();
    m_taskbarProgress->SetWindow(m_view->GetWindowHandle());

    const auto progressHandler = [this](const auto& progress) {
        ReportProgressToStatusBar(progress);
        ReportProgressToTaskbar(progress, *m_taskbarProgress);
    };

    const auto completionHandler = [this](const auto& progress, const auto& scanningResults) {
        OnScanComplete(progress, scanningResults);
    };

    const auto& log = spdlog::get(Constants::Logging::DefaultLog);
    log->info("Started a new scan at \"{}\".", m_model->GetRootPath().string());

    m_scanner.StartScanning(ScanningOptions{ root, progressHandler, completionHandler });
}

void Controller::StopScanning()
{
    if (m_scanner.IsActive()) {
        m_scanner.StopProgressReporting();
        m_scanner.StopScanning();
    }
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
            "Time Elapsed: {:02n}:{:02n}:{:02n}  |  Files Scanned: {:n}  |  {:03.2f}% Complete",
            hours, minutes, seconds, filesScanned, fraction * 100);

        m_view->SetStatusBarMessage(message);
    } else {
        const auto prefix = m_sessionSettings.GetActiveNumericPrefix();
        const auto [size, units] = Utilities::ToPrefixedSize(sizeInBytes, prefix);

        const auto message = fmt::format(
            "Time Elapsed: {:02n}:{:02n}:{:02n}  |  Files Scanned: {:n}  |  {:03.2f} {} and "
            "counting...",
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
    const auto isSmallFile = units.find(Constants::Units::Bytes) != std::string::npos;

    const auto path = Controller::NodeToFilePath(node).string();
    const auto message = isSmallFile ? fmt::format("{}  |  {:.0f} {}", path, prefixedSize, units)
                                     : fmt::format("{}  |  {:.2f} {}", path, prefixedSize, units);

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

    const auto& options = m_sessionSettings.GetVisualizationOptions();
    const auto* node = m_model->FindNearestIntersection(camera, ray, options);

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
        "Scanned {:n} files and {:n} directories.", metadata.FileCount, metadata.DirectoryCount);

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
    const auto isSmallFile = units.find(Constants::Units::Bytes) != std::string::npos;

    const std::string nodes = highlightedNodes.size() == 1 ? " node" : " nodes";

    if (isSmallFile) {
        m_view->SetStatusBarMessage(fmt::format(
            "Highlighted {:n} " + nodes + ", presenting {:.0f} {}.", highlightedNodes.size(),
            prefixedSize, units));
    } else {
        m_view->SetStatusBarMessage(fmt::format(
            "Highlighted {:n} " + nodes + ", presenting {:.2f} {}.", highlightedNodes.size(),
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
        m_model->HighlightDescendants(node, m_sessionSettings.GetVisualizationOptions());
    };

    ProcessHighlightedNodes(selector, callback);
}

void Controller::HighlightAllMatchingExtensions(
    const std::string& extension,
    const std::function<void(std::vector<const Tree<VizBlock>::Node*>&)>& callback)
{
    Expects(m_model);

    const auto& options = m_sessionSettings.GetVisualizationOptions();
    const auto selector = [&] { m_model->HighlightMatchingFileExtensions(extension, options); };

    ProcessHighlightedNodes(selector, callback);
}

void Controller::SearchTreeMap(
    const std::string& searchQuery,
    const std::function<void(std::vector<const Tree<VizBlock>::Node*>&)>& deselectionCallback,
    const std::function<void(std::vector<const Tree<VizBlock>::Node*>&)>& selectionCallback,
    SearchFlags flags)
{
    const auto shouldSearchFiles = flags & SearchFlags::SearchFiles;
    const auto shouldSearchDirectories = flags & SearchFlags::SearchDirectories;

    if (searchQuery.empty() || !HasModelBeenLoaded() ||
        (!shouldSearchFiles && !shouldSearchDirectories)) {
        return;
    }

    ClearHighlightedNodes(deselectionCallback);

    const auto selector = [&] {
        const auto stopwatch = Stopwatch<std::chrono::milliseconds>([&]() noexcept {
            m_model->HighlightMatchingFileNames(
                searchQuery, m_sessionSettings.GetVisualizationOptions(), flags);
        });

        const auto& log = spdlog::get(Constants::Logging::DefaultLog);
        log->info(
            "Search Completed in: {:n} {}.", stopwatch.GetElapsedTime().count(),
            stopwatch.GetUnitsAsString());
    };

    ProcessHighlightedNodes(selector, selectionCallback);
}

std::filesystem::path Controller::NodeToFilePath(const Tree<VizBlock>::Node& node)
{
    std::vector<std::reference_wrapper<const std::string>> reversePath;
    reversePath.reserve(Tree<VizBlock>::Depth(node));
    reversePath.emplace_back(node->file.name);

    const auto* currentNode = &node;
    while (currentNode->GetParent()) {
        currentNode = currentNode->GetParent();
        reversePath.emplace_back(currentNode->GetData().file.name);
    }

    const auto completePath = std::accumulate(
        std::rbegin(reversePath), std::rend(reversePath), std::string{},
        [](const std::string& path, const std::string& file) {
            constexpr auto slash = '/';

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
void Controller::ReportProgressToTaskbar(const ScanningProgress& progress, ButtonType& button)
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
