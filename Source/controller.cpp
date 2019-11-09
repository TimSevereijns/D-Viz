#include "controller.h"

#include "Settings/settingsManager.h"
#include "Utilities/ignoreUnused.hpp"
#include "Utilities/operatingSystemSpecific.hpp"
#include "Utilities/scopeExit.hpp"
#include "Visualizations/squarifiedTreemap.h"
#include "Windows/mainWindow.h"
#include "constants.h"
#include "literals.h"

#include <Stopwatch/Stopwatch.hpp>
#include <gsl/gsl_assert>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <sstream>
#include <utility>

#include <QCursor>

#if defined(Q_OS_WIN)
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>

#include "Scanner/Monitor/windowsFileMonitor.h"
#elif defined(Q_OS_LINUX)
#include "Scanner/Monitor/linuxFileMonitor.h"
#include "Windows/nullTaskbarButton.h"
#endif // Q_OS_LINUX

namespace
{
#if defined(Q_OS_WIN)
    using FileSystemMonitor = WindowsFileMonitor;
    using TaskbarButton = QWinTaskbarButton;
#elif defined(Q_OS_LINUX)
    using FileSystemMonitor = LinuxFileMonitor;
    using TaskbarButton = NullTaskbarButton;
#endif // Q_OS_LINUX

    constexpr const std::wstring_view bytesLabel{ L" bytes" };

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

        if (sizeInBytes < 1_KiB) {
            return std::make_pair(sizeInBytes, std::wstring{ bytesLabel });
        }

        if (sizeInBytes < 1_MiB) {
            return std::make_pair(sizeInBytes / 1_KiB, L" KiB");
        }

        if (sizeInBytes < 1_GiB) {
            return std::make_pair(sizeInBytes / 1_MiB, L" MiB");
        }

        if (sizeInBytes < 1_TiB) {
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

        if (sizeInBytes < 1_KB) {
            return std::make_pair(sizeInBytes, std::wstring{ bytesLabel });
        }

        if (sizeInBytes < 1_MB) {
            return std::make_pair(sizeInBytes / 1_KB, L" KB");
        }

        if (sizeInBytes < 1_GB) {
            return std::make_pair(sizeInBytes / 1_MB, L" MB");
        }

        if (sizeInBytes < 1_TB) {
            return std::make_pair(sizeInBytes / 1_GB, L" GB");
        }

        return std::make_pair(sizeInBytes / 1_TB, L" TB");
    }

    /**
     * @returns The full path to the JSON file that contains the color mapping.
     */
    std::filesystem::path GetColorJsonPath()
    {
        return std::filesystem::current_path().append(L"colors.json");
    }

    /**
     * @returns The full path to the JSON file that contains the user preferences.
     */
    std::filesystem::path GetPreferencesJsonPath()
    {
        return std::filesystem::current_path().append(L"preferences.json");
    }

    /**
     * @brief Helper function to be called once scanning completes.
     *
     * @param[in] progress           The final results from the scan.
     */
    void LogScanCompletion(const ScanningProgress& progress)
    {
        const auto& log = spdlog::get(Constants::Logging::DefaultLog);

        log->info(fmt::format(
            "Scanned: {} directories and {} files, representing {} bytes",
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
        log->info(fmt::format("Disk Size:  {} bytes", spaceInfo.capacity));
        log->info(fmt::format("Free Space: {} bytes", spaceInfo.free));
    }
} // namespace

Controller::Controller()
    : m_settingsManager{ GetColorJsonPath(), GetPreferencesJsonPath() },
      m_view{ std::make_unique<MainWindow>(*this) }
{
}

void Controller::LaunchUI()
{
    m_view->show();
}

void Controller::OnScanComplete(
    const Settings::VisualizationParameters& parameters, const ScanningProgress& progress,
    const std::shared_ptr<Tree<VizBlock>>& scanningResults)
{
    WriteProgressToStatusBar(progress);
    LogScanCompletion(progress);

    m_view->AskUserToLimitFileSize(progress.filesScanned.load(), parameters);

    m_view->SetWaitCursor();
    const ScopeExit onScopeExit = [&]() noexcept
    {
        m_view->RestoreDefaultCursor();
    };

    m_model->Parse(scanningResults);
    m_model->UpdateBoundingBoxes();

    SaveScanMetadata(progress);

    m_view->OnScanCompleted();
    m_model->StartMonitoringFileSystem();

    AllowUserInteractionWithModel(true);
}

void Controller::ScanDrive(const Settings::VisualizationParameters& parameters)
{
    AllowUserInteractionWithModel(false);

    m_model = std::make_unique<SquarifiedTreeMap>(
        std::make_unique<FileSystemMonitor>(), parameters.rootDirectory);

    m_view->OnScanStarted();

    const auto spaceInfo = std::filesystem::space(parameters.rootDirectory);
    LogDiskStatistics(spaceInfo);
    m_occupiedDiskSpace = spaceInfo.capacity - spaceInfo.free;
    Expects(m_occupiedDiskSpace > 0u);

    auto button = std::make_shared<TaskbarButton>(m_view.get());
    button->setWindow(m_view->windowHandle());

    const auto progressHandler = [&, button](const ScanningProgress& progress) {
        WriteProgressToStatusBar(progress);
        ReportProgressToTaskbar(*button, progress);
    };

    const auto completionHandler =
        [&, button](
            const ScanningProgress& progress,
            const std::shared_ptr<Tree<VizBlock>>& scanningResults) mutable {
            OnScanComplete(parameters, progress, scanningResults);

            button->progress()->reset();
            button.reset();
        };

    spdlog::get(Constants::Logging::DefaultLog)
        ->info(fmt::format("Started a new scan at: \"{}\"", m_model->GetRootPath().string()));

    m_scanner.StartScanning(
        ScanningParameters{ parameters.rootDirectory, progressHandler, completionHandler });
}

bool Controller::IsFileSystemBeingMonitored() const
{
    return m_model->IsFileSystemBeingMonitored();
}

std::optional<FileEvent> Controller::FetchFileModification()
{
    return m_model->FetchNextVisualChange();
}

QVector3D Controller::DetermineNodeColor(const Tree<VizBlock>::Node& node) const
{
    Expects(node.GetData().offsetIntoVBO != VizBlock::INVALID_OFFSET);

    const auto nodeColor = m_nodeColorMap.find(node.GetData().offsetIntoVBO);
    if (nodeColor != std::end(m_nodeColorMap)) {
        return nodeColor->second;
    }

    if (IsNodeHighlighted(node)) {
        return Constants::Colors::SlateGray;
    }

    if (m_settingsManager.GetActiveColorScheme() != Constants::ColorScheme::Default) {
        const auto fileColor = m_settingsManager.DetermineColorFromExtension(node);
        if (fileColor) {
            return *fileColor;
        }
    }

    if (node->file.type == FileType::DIRECTORY) {
        return Constants::Colors::White;
    }

    Expects(node->file.type == FileType::REGULAR);
    return Constants::Colors::FileGreen;
}

void Controller::WriteProgressToStatusBar(const ScanningProgress& progress)
{
    Expects(m_occupiedDiskSpace > 0u);

    const auto filesScanned = progress.filesScanned.load();
    const auto sizeInBytes = progress.bytesProcessed.load();

    const auto rootPath = m_model->GetRootPath();
    const auto doesPathRepresentEntireDrive{ rootPath == rootPath.root_path() };

    if (doesPathRepresentEntireDrive) {
        const auto fraction =
            (static_cast<double>(sizeInBytes) / static_cast<double>(m_occupiedDiskSpace));

        const auto message = fmt::format(
            L"Files Scanned: {}  |  {:03.2f}% Complete",
            Utilities::StringifyWithDigitSeparators(filesScanned), fraction * 100);

        m_view->SetStatusBarMessage(message);
    } else {
        const auto prefix = m_settingsManager.GetActiveNumericPrefix();
        const auto [size, units] = Controller::ConvertFileSizeToNumericPrefix(sizeInBytes, prefix);

        const auto message = fmt::format(
            L"Files Scanned: {}  |  {:03.2f} {} and counting...",
            Utilities::StringifyWithDigitSeparators(filesScanned), size, units);

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
    const auto prefix = m_settingsManager.GetActiveNumericPrefix();
    const auto [prefixedSize, units] = ConvertFileSizeToNumericPrefix(fileSize, prefix);
    const auto isInBytes = (units == bytesLabel);

    const auto path = Controller::ResolveCompleteFilePath(node).wstring();

    std::wstringstream message;
    message.imbue(std::locale{ "" });
    message.precision(isInBytes ? 0 : 2);
    message << std::fixed << path << L"  |  " << prefixedSize << units;

    m_view->SetStatusBarMessage(message.str());
}

void Controller::SelectNodeViaRay(
    const Camera& camera, const Ray& ray,
    const std::function<void(const Tree<VizBlock>::Node&)>& deselectionCallback,
    const std::function<void(const Tree<VizBlock>::Node&)>& selectionCallback)
{
    Expects(m_model);

    if (!HasModelBeenLoaded() || !IsUserAllowedToInteractWithModel()) {
        return;
    }

    const auto* const selectedNode = m_model->GetSelectedNode();

    if (selectedNode) {
        deselectionCallback(*selectedNode);
        m_model->ClearSelectedNode();
    }

    const auto& parameters = m_settingsManager.GetVisualizationParameters();
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

    std::wstringstream message;
    message.imbue(std::locale{ "" });
    message << std::fixed << L"Scanned " << metadata.FileCount << L" files and "
            << metadata.DirectoryCount << L" directories.";

    m_view->SetStatusBarMessage(message.str());
}

void Controller::DisplaySelectionDetails()
{
    const auto& highlightedNodes = m_model->GetHighlightedNodes();

    std::uintmax_t totalBytes{ 0 };
    for (const auto* const node : highlightedNodes) {
        totalBytes += node->GetData().file.size;
    }

    const auto prefix = m_settingsManager.GetActiveNumericPrefix();
    const auto [prefixedSize, units] = ConvertFileSizeToNumericPrefix(totalBytes, prefix);
    const auto isInBytes = (units == bytesLabel);

    std::wstringstream message;
    message.imbue(std::locale{ "" });
    message.precision(isInBytes ? 0 : 2);
    message << std::fixed << L"Highlighted " << highlightedNodes.size()
            << (highlightedNodes.size() == 1 ? L" node" : L" nodes") << L", representing "
            << prefixedSize << units;

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

    DisplaySelectionDetails();
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
        m_model->HighlightDescendants(node, m_settingsManager.GetVisualizationParameters());
    };

    ProcessHighlightedNodes(selector, callback);
}

void Controller::HighlightAllMatchingExtensions(
    const Tree<VizBlock>::Node& sampleNode,
    const std::function<void(std::vector<const Tree<VizBlock>::Node*>&)>& callback)
{
    Expects(m_model);

    const auto& parameters = m_settingsManager.GetVisualizationParameters();
    const auto selector = [&] {
        m_model->HighlightMatchingFileExtensions(sampleNode.GetData().file.extension, parameters);
    };

    ProcessHighlightedNodes(selector, callback);
}

void Controller::SearchTreeMap(
    const std::wstring& searchQuery,
    const std::function<void(std::vector<const Tree<VizBlock>::Node*>&)>& deselectionCallback,
    const std::function<void(std::vector<const Tree<VizBlock>::Node*>&)>& selectionCallback,
    bool shouldSearchFiles, bool shouldSearchDirectories)
{
    Expects(m_model);

    if (searchQuery.empty() || !HasModelBeenLoaded() ||
        (!shouldSearchFiles && !shouldSearchDirectories)) {
        return;
    }

    ClearHighlightedNodes(deselectionCallback);

    const auto selector = [&] {
        Stopwatch<std::chrono::milliseconds>(
            [&]() noexcept {
                m_model->HighlightMatchingFileNames(
                    searchQuery, m_settingsManager.GetVisualizationParameters(), shouldSearchFiles,
                    shouldSearchDirectories);
            },
            [](const auto& elapsed, const auto& units) noexcept {
                spdlog::get(Constants::Logging::DefaultLog)
                    ->info(fmt::format("Search Completed in: {} {}", elapsed.count(), units));
            });
    };

    ProcessHighlightedNodes(selector, selectionCallback);
}

std::pair<double, std::wstring> Controller::ConvertFileSizeToNumericPrefix(
    std::uintmax_t sizeInBytes, Constants::FileSize::Prefix prefix)
{
    switch (prefix) {
        case Constants::FileSize::Prefix::BINARY: {
            return ConvertToBinaryPrefix(sizeInBytes);
        }
        case Constants::FileSize::Prefix::DECIMAL: {
            return ConvertToDecimalPrefix(sizeInBytes);
        }
    }

    GSL_ASSUME(false);
}

std::filesystem::path Controller::ResolveCompleteFilePath(const Tree<VizBlock>::Node& node)
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

Settings::Manager& Controller::GetSettingsManager()
{
    return m_settingsManager;
}

const Settings::Manager& Controller::GetSettingsManager() const
{
    return m_settingsManager;
}

std::filesystem::path Controller::GetRootPath() const
{
    return m_model->GetRootPath();
}

void Controller::RegisterNodeColor(const Tree<VizBlock>::Node& node, const QVector3D& color)
{
    Expects(node.GetData().offsetIntoVBO != VizBlock::INVALID_OFFSET);
    m_nodeColorMap.insert_or_assign(node.GetData().offsetIntoVBO, color);
}

template <typename ButtonType>
void Controller::ReportProgressToTaskbar(ButtonType& button, const ScanningProgress& progress)
{
    IgnoreUnused(button, progress);

#if defined(Q_OS_WIN)
    const auto sizeInBytes = progress.bytesProcessed.load();

    const auto rootPath = m_model->GetRootPath();
    const auto doesPathRepresentEntireDrive{ rootPath == rootPath.root_path() };

    if (doesPathRepresentEntireDrive) {
        const auto progressValue =
            (static_cast<double>(sizeInBytes) / static_cast<double>(m_occupiedDiskSpace));

        QWinTaskbarProgress* const progressOverlay = button.progress();
        progressOverlay->setVisible(true);
        progressOverlay->setValue(static_cast<int>(100.0 * progressValue));
    } else {
        QWinTaskbarProgress* const progressOverlay = button.progress();
        progressOverlay->setVisible(true);
        progressOverlay->setMinimum(0);
        progressOverlay->setMaximum(0);
    }
#endif // Q_OS_WIN
}
