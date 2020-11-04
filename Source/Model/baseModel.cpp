#include "Model/baseModel.h"
#include "Model/Monitor/fileChangeNotification.h"
#include "Model/Scanner/scanningUtilities.h"
#include "Model/ray.h"
#include "Utilities/utilities.h"
#include "constants.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <Stopwatch/Stopwatch.hpp>
#include <gsl/gsl_assert>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <chrono>
#include <optional>
#include <string>

#include <QColor>
#include <QRectF>

namespace
{
    namespace Normals
    {
        [[maybe_unused]] constexpr QVector3D PositiveX{ 1.0f, 0.0f, 0.0f };
        [[maybe_unused]] constexpr QVector3D PositiveY{ 0.0f, 1.0f, 0.0f };
        [[maybe_unused]] constexpr QVector3D PositiveZ{ 0.0f, 0.0f, 1.0f };
        [[maybe_unused]] constexpr QVector3D NegativeX{ -1.0f, 0.0f, 0.0f };
        [[maybe_unused]] constexpr QVector3D NegativeY{ 0.0f, -1.0f, 0.0f };
        [[maybe_unused]] constexpr QVector3D NegativeZ{ 0.0f, 0.0f, -1.0f };
    } // namespace Normals

    /**
     * @brief Calculates whether the specified ray hits the specified plane, given a margin of
     * error,
     * epsilon.
     *
     * @param[in] ray                The ray to be fired at the plane.
     * @param[in] pointOnPlane       Any point on the plane.
     * @param[in] planeNormal        The normal for that point on the plane.
     *
     * @returns The point of intersection if there is an intersection greater than the margin of
     * error, or boost::none if no such intersection exists.
     */
    std::optional<QVector3D> DoesRayIntersectPlane(
        const Ray& ray, const QVector3D& pointOnPlane, const QVector3D& planeNormal)
    {
        constexpr auto epsilon{ 0.0001f };

        const auto denominator = QVector3D::dotProduct(ray.Direction(), planeNormal);
        if (std::abs(denominator) < epsilon) {
            return std::nullopt;
        }

        const auto numerator = QVector3D::dotProduct(pointOnPlane - ray.Origin(), planeNormal);

        const auto scalar = numerator / denominator;
        const bool doesRayHitPlane = std::abs(scalar) > epsilon;

        if (!doesRayHitPlane) {
            return std::nullopt;
        }

        return scalar * ray.Direction().normalized() + ray.Origin();
    }

    /**
     * @brief Returns the intersection point that is closest to the origin of the ray.
     *
     * @param[in] ray                The ray that caused the intersections.
     * @param[in] intersections      All the intersections caused by the ray.
     *
     * @return The closest intersection point, or boost::none should anything weird occur.
     */
    std::optional<QVector3D>
    FindClosestIntersectionPoint(const Ray& ray, const std::vector<QVector3D>& allIntersections)
    {
        const auto& closest = std::min_element(
            std::begin(allIntersections),
            std::end(allIntersections), [&ray](const auto& lhs, const auto& rhs) noexcept {
                return (ray.Origin().distanceToPoint(lhs) < ray.Origin().distanceToPoint(rhs));
            });

        if (closest == std::end(allIntersections)) {
            return std::nullopt;
        }

        return *closest;
    }

    /**
     * @brief Finds the point at which the given ray intersects the given block.
     *
     * @param[in] ray                The ray fired at the block.
     * @param[in] block              The block to be tested for intersection.
     *
     * @returns The point of intersection should it exist; boost::none otherwise.
     */
    std::optional<QVector3D> DoesRayIntersectBlock(const Ray& ray, const Block& block)
    {
        std::vector<QVector3D> allIntersections;

        const auto blockOrigin = block.GetOrigin();
        const auto blockHeight = block.GetHeight();
        const auto blockWidth = block.GetWidth();
        const auto blockDepth = block.GetDepth();

        { // Perform hit detection on the top face:
            const auto randomPointOnTopFace = blockOrigin + PrecisePoint{ 0, blockHeight, 0 };
            const QVector3D pointOnPlane{ static_cast<float>(randomPointOnTopFace.x()),
                                          static_cast<float>(randomPointOnTopFace.y()),
                                          static_cast<float>(randomPointOnTopFace.z()) };

            const std::optional<QVector3D> intersectionPoint =
                DoesRayIntersectPlane(ray, pointOnPlane, Normals::PositiveY);

            // clang-format off
            if (intersectionPoint && blockOrigin.xAsFloat()             < intersectionPoint->x() &&
                blockOrigin.xAsFloat() + static_cast<float>(blockWidth) > intersectionPoint->x() &&
                blockOrigin.zAsFloat()                                  > intersectionPoint->z() &&
                blockOrigin.zAsFloat() - static_cast<float>(blockDepth) < intersectionPoint->z()) {
                allIntersections.emplace_back(*intersectionPoint);
            }
            // clang-format on
        }

        { // Perform hit detection on the front face:
            const auto randomPointOnFrontFace = blockOrigin;
            const QVector3D pointOnPlane{ static_cast<float>(randomPointOnFrontFace.x()),
                                          static_cast<float>(randomPointOnFrontFace.y()),
                                          static_cast<float>(randomPointOnFrontFace.z()) };

            const std::optional<QVector3D> intersectionPoint =
                DoesRayIntersectPlane(ray, pointOnPlane, Normals::PositiveZ);

            // clang-format off
            if (intersectionPoint && blockOrigin.xAsFloat()              < intersectionPoint->x() &&
                blockOrigin.xAsFloat() + static_cast<float>(blockWidth)  > intersectionPoint->x() &&
                blockOrigin.yAsFloat()                                   < intersectionPoint->y() &&
                blockOrigin.yAsFloat() + static_cast<float>(blockHeight) > intersectionPoint->y()) {
                allIntersections.emplace_back(*intersectionPoint);
            }
            // clang-format on
        }

        { // Perform hit detection on the back face:
            const auto randomPointOnBackFace = blockOrigin + PrecisePoint{ 0, 0, -blockDepth };
            const QVector3D pointOnPlane{ static_cast<float>(randomPointOnBackFace.x()),
                                          static_cast<float>(randomPointOnBackFace.y()),
                                          static_cast<float>(randomPointOnBackFace.z()) };

            const std::optional<QVector3D> intersectionPoint =
                DoesRayIntersectPlane(ray, pointOnPlane, Normals::NegativeZ);

            // clang-format off
            if (intersectionPoint && blockOrigin.xAsFloat()              < intersectionPoint->x() &&
                blockOrigin.xAsFloat() + static_cast<float>(blockWidth)  > intersectionPoint->x() &&
                blockOrigin.yAsFloat()                                   < intersectionPoint->y() &&
                blockOrigin.yAsFloat() + static_cast<float>(blockHeight) > intersectionPoint->y()) {
                allIntersections.emplace_back(*intersectionPoint);
            }
            // clang-format on
        }

        { // Perform hit detection on the left face:
            const auto randomPointOnLeftFace = blockOrigin;
            const QVector3D pointOnPlane{ static_cast<float>(randomPointOnLeftFace.x()),
                                          static_cast<float>(randomPointOnLeftFace.y()),
                                          static_cast<float>(randomPointOnLeftFace.z()) };

            const std::optional<QVector3D> intersectionPoint =
                DoesRayIntersectPlane(ray, pointOnPlane, Normals::NegativeX);

            // clang-format off
            if (intersectionPoint && blockOrigin.zAsFloat()              > intersectionPoint->z() &&
                blockOrigin.zAsFloat() - static_cast<float>(blockDepth)  < intersectionPoint->z() &&
                blockOrigin.yAsFloat()                                   < intersectionPoint->y() &&
                blockOrigin.yAsFloat() + static_cast<float>(blockHeight) > intersectionPoint->y()) {
                allIntersections.emplace_back(*intersectionPoint);
            }
            // clang-format on
        }

        { // Perform hit detection on the right face:
            const auto randomPointOnRightFace = blockOrigin + PrecisePoint{ blockWidth, 0, 0 };
            const QVector3D pointOnPlane{ static_cast<float>(randomPointOnRightFace.x()),
                                          static_cast<float>(randomPointOnRightFace.y()),
                                          static_cast<float>(randomPointOnRightFace.z()) };

            const std::optional<QVector3D> intersectionPoint =
                DoesRayIntersectPlane(ray, pointOnPlane, Normals::PositiveX);

            // clang-format off
            if (intersectionPoint && blockOrigin.zAsFloat()              > intersectionPoint->z() &&
                blockOrigin.zAsFloat() - static_cast<float>(blockDepth)  < intersectionPoint->z() &&
                blockOrigin.yAsFloat()                                   < intersectionPoint->y() &&
                blockOrigin.yAsFloat() + static_cast<float>(blockHeight) > intersectionPoint->y()) {
                allIntersections.emplace_back(*intersectionPoint);
            }
            // clang-format on
        }

        return FindClosestIntersectionPoint(ray, allIntersections);
    }

    /**
     * @brief Helper function that will advance the passed-in node to the next node in the tree that
     * is not a descendant of said node.
     *
     * @param[out] node              The node to advance.
     */
    void AdvanceToNextNonDescendant(Tree<VizBlock>::Node*& node)
    {
        if (node->GetNextSibling()) {
            node = node->GetNextSibling();
        } else {
            while (node->GetParent() && !node->GetParent()->GetNextSibling()) {
                node = node->GetParent();
            }

            node = node->GetParent() ? node->GetParent()->GetNextSibling() : nullptr;
        }
    }

    /**
     * @brief Represents the point at which a node intersection occured, as well as the node that
     * was hit.
     */
    struct IntersectionInfo
    {
        QVector3D point;
        Tree<VizBlock>::Node* node;
    };

    /**
     * @brief Iterates over all nodes in the scene, placing all intersections in a vector.
     *
     * @param[in] ray                The ray to be shot into the scene.
     * @param[in] camera             The camera from which the ray is shot.
     * @param[in] parameters         Additional visualization parameters.
     * @param[in] node               The current node being hit-tested.
     */
    std::vector<IntersectionInfo> FindAllIntersections(
        const Ray& ray, const Camera& camera, const Settings::VisualizationParameters& parameters,
        Tree<VizBlock>::Node* node)
    {
        Expects(node != nullptr);

        std::vector<IntersectionInfo> allIntersections;

        while (node) {
            if (!parameters.IsNodeVisible(node->GetData())) {
                AdvanceToNextNonDescendant(node);
                continue;
            }

            if (DoesRayIntersectBlock(ray, node->GetData().boundingBox)) {
                const auto& blockIntersection = DoesRayIntersectBlock(ray, node->GetData().block);

                if (blockIntersection && camera.IsPointInFrontOfCamera(*blockIntersection)) {
                    allIntersections.emplace_back(IntersectionInfo{ *blockIntersection, node });
                }

                if (node->HasChildren()) {
                    node = node->GetFirstChild();
                } else {
                    AdvanceToNextNonDescendant(node);
                }
            } else {
                AdvanceToNextNonDescendant(node);
            }
        }

        return allIntersections;
    }

    /**
     * @brief Logs filesystem changes.
     */
    void LogFileSystemEvent(const FileEvent& event)
    {
        switch (event.eventType) {
            case FileEventType::Created:
                spdlog::get(Constants::Logging::FilesystemLog)
                    ->info(fmt::format("Created: {}", event.path.string()));
                break;
            case FileEventType::Deleted:
                spdlog::get(Constants::Logging::FilesystemLog)
                    ->info(fmt::format("Deleted: {}", event.path.string()));
                break;
            case FileEventType::Touched:
                spdlog::get(Constants::Logging::FilesystemLog)
                    ->info(fmt::format("Modified: {}", event.path.string()));
                break;
            case FileEventType::Renamed:
                spdlog::get(Constants::Logging::FilesystemLog)
                    ->info(fmt::format("Renamed: {}", event.path.string()));
                break;
            default:
                GSL_ASSUME(false);
        }
    }
} // namespace

BaseModel::BaseModel(
    std::unique_ptr<FileMonitorBase> fileMonitor, const std::filesystem::path& path)
    : m_rootPath{ path }, m_fileSystemObserver{ std::move(fileMonitor), path }
{
}

BaseModel::~BaseModel() noexcept
{
    StopMonitoringFileSystem();
}

void BaseModel::UpdateBoundingBoxes()
{
    Expects(m_hasDataBeenParsed == true);
    Expects(m_fileTree != nullptr);

    if (!m_hasDataBeenParsed) {
        return;
    }

    std::for_each(std::begin(*m_fileTree), std::end(*m_fileTree), [](auto& node) noexcept {
        if (!node.HasChildren()) {
            node->boundingBox = node->block;
            return;
        }

        double tallestDescendant = 0.0;

        auto* currentChild = node.GetFirstChild();
        while (currentChild) {
            if (currentChild->GetData().boundingBox.GetHeight() > tallestDescendant) {
                tallestDescendant = currentChild->GetData().boundingBox.GetHeight();
            }

            currentChild = currentChild->GetNextSibling();
        }

        node->boundingBox =
            Block{ node->block.GetOrigin(), node->block.GetWidth(),
                   node->block.GetHeight() + tallestDescendant, node->block.GetDepth() };
    });
}

Tree<VizBlock>::Node* BaseModel::FindNearestIntersection(
    const Camera& camera, const Ray& ray, const Settings::VisualizationParameters& parameters) const
{
    if (!m_hasDataBeenParsed) {
        return nullptr;
    }

    Tree<VizBlock>::Node* nearestIntersection = nullptr;

    const auto stopwatch = Stopwatch<std::chrono::microseconds>([&]() noexcept {
        const auto root = m_fileTree->GetRoot();
        const auto intersections = FindAllIntersections(ray, camera, parameters, root);

        if (intersections.empty()) {
            return;
        }

        const auto closest = std::min_element(
            std::begin(intersections),
            std::end(intersections), [&ray](const auto& lhs, const auto& rhs) noexcept {
                return (
                    ray.Origin().distanceToPoint(lhs.point) <
                    ray.Origin().distanceToPoint(rhs.point));
            });

        nearestIntersection = closest->node;
    });

    spdlog::get(Constants::Logging::DefaultLog)
        ->info(fmt::format(
            "Selected node in: {:n} {}", stopwatch.GetElapsedTime().count(),
            stopwatch.GetUnitsAsCharacterArray()));

    return nearestIntersection;
}

Tree<VizBlock>& BaseModel::GetTree()
{
    Expects(m_fileTree != nullptr);

    return *m_fileTree;
}

const Tree<VizBlock>& BaseModel::GetTree() const
{
    Expects(m_fileTree != nullptr);

    return *m_fileTree;
}

const std::vector<const Tree<VizBlock>::Node*>& BaseModel::GetHighlightedNodes() const
{
    return m_highlightedNodes;
}

std::vector<const Tree<VizBlock>::Node*>& BaseModel::GetHighlightedNodes()
{
    return m_highlightedNodes;
}

void BaseModel::ClearHighlightedNodes()
{
    if (m_highlightedNodes.size() == 0) {
        return;
    }

    m_highlightedNodes.clear();
}

void BaseModel::SelectNode(const Tree<VizBlock>::Node& node)
{
    m_selectedNode = &node;
}

const Tree<VizBlock>::Node* BaseModel::GetSelectedNode()
{
    return m_selectedNode;
}

void BaseModel::ClearSelectedNode()
{
    m_selectedNode = nullptr;
}

TreemapMetadata BaseModel::GetTreemapMetadata()
{
    return m_metadata;
}

void BaseModel::SetTreemapMetadata(TreemapMetadata&& data)
{
    m_metadata = data;
}

void BaseModel::HighlightAncestors(const Tree<VizBlock>::Node& node)
{
    auto* currentNode = node.GetParent();
    while (currentNode) {
        m_highlightedNodes.emplace_back(currentNode);

        currentNode = currentNode->GetParent();
    }
}

void BaseModel::HighlightDescendants(
    const Tree<VizBlock>::Node& root, const Settings::VisualizationParameters& parameters)
{
    std::for_each(
        Tree<VizBlock>::LeafIterator{ &root }, Tree<VizBlock>::LeafIterator{},
        [&](const auto& node) {
            if ((parameters.onlyShowDirectories && node->file.type != FileType::Directory) ||
                node->file.size < parameters.minimumFileSize) {
                return;
            }

            HighlightNode(&node);
        });
}

void BaseModel::HighlightMatchingFileExtensions(
    const std::wstring& extension, const Settings::VisualizationParameters& parameters)
{
    std::for_each(
        Tree<VizBlock>::LeafIterator{ GetTree().GetRoot() }, Tree<VizBlock>::LeafIterator{},
        [&](const auto& node) {
            if ((parameters.onlyShowDirectories && node->file.type != FileType::Directory) ||
                node->file.size < parameters.minimumFileSize || node->file.extension != extension) {
                return;
            }

            HighlightNode(&node);
        });
}

void BaseModel::HighlightMatchingFileNames(
    const std::wstring& searchQuery, const Settings::VisualizationParameters& parameters,
    bool shouldSearchFiles, bool shouldSearchDirectories)
{
    std::wstring fileAndExtension;
    fileAndExtension.resize(260); ///< Resize to prevent reallocation with append operations.

    const auto lowercaseQuery = boost::algorithm::to_lower_copy(searchQuery);

    std::for_each(
        Tree<VizBlock>::PostOrderIterator{ GetTree().GetRoot() },
        Tree<VizBlock>::PostOrderIterator{}, [&](const auto& node) {
            const auto& file = node->file;

            if (file.size < parameters.minimumFileSize ||
                (!shouldSearchDirectories && file.type == FileType::Directory) ||
                (!shouldSearchFiles && file.type == FileType::Regular)) {
                return;
            }

            fileAndExtension = file.name;
            fileAndExtension.append(file.extension);

            boost::algorithm::to_lower(fileAndExtension);

            // @note We're converting everything to lowercase beforehand
            // (instead of using `boost::icontains(...)`), since doing so is significantly faster.
            if (!boost::contains(fileAndExtension, lowercaseQuery)) {
                return;
            }

            HighlightNode(&node);
        });
}

void BaseModel::HighlightNode(const Tree<VizBlock>::Node* const node)
{
    Expects(node != nullptr);

    m_highlightedNodes.emplace_back(node);
}

void BaseModel::StartMonitoringFileSystem()
{
    const auto callback = [&](FileEvent && event) noexcept
    {
        m_fileEvents.Emplace(std::move(event));
    };

    m_fileEvents.ResetWaitingPrivileges();
    m_shouldKeepProcessingNotifications.store(true, std::memory_order::memory_order_relaxed);

    m_fileSystemObserver.StartMonitoring(callback);
    m_fileSystemNotificationProcessor = std::thread{ [&] { ProcessChanges(); } };
}

void BaseModel::StopMonitoringFileSystem() noexcept

{
    m_fileSystemObserver.StopMonitoring();

    m_shouldKeepProcessingNotifications.store(false);
    m_fileEvents.AbandonWait();

    if (m_fileSystemNotificationProcessor.joinable()) {
        m_fileSystemNotificationProcessor.join();
    }
}

void BaseModel::ProcessChanges()
{
    while (m_shouldKeepProcessingNotifications.load()) {
        const auto event = m_fileEvents.WaitAndPop();
        if (!event) {
            // @note If we got here, it may indicates that the wait operation has probably been
            // abandoned due to a DTOR invocation.
            continue;
        }

        LogFileSystemEvent(*event);

        // @todo Should there be an upper limit on the number of changes that can be in the
        // queue at any given time?

        m_pendingVisualUpdates.Push(*event);
        m_pendingModelUpdates.Push(*event);

        m_eventNotificationReady.notify_one();
    }
}

void BaseModel::WaitForNextModelChange()
{
    std::unique_lock<std::mutex> lock{ m_eventNotificationMutex };
    m_eventNotificationReady.wait(lock, [&]() { return !m_pendingModelUpdates.IsEmpty(); });
}

void BaseModel::RefreshTreemap()
{
    auto fileEvent = FetchNextModelChange();

    while (fileEvent) {
        UpdateAffectedNodes(*fileEvent);
        fileEvent = FetchNextModelChange();
    }

    // @todo Sort the tree.
    // @todo Update all sizes.
}

void BaseModel::UpdateAffectedNodes(const FileEvent& event)
{
    const auto absolutePath = event.path;

    std::error_code errorCode;

    if (event.eventType == FileEventType::Touched && !std::filesystem::exists(absolutePath) &&
        !errorCode) {
        // @note The absence of a file may not necessarily indicate a bug, since there tend to be
        // a lot of transient files that may only exist for a fraction of a second. For example,
        // some applications tend to create temporary files when saving changes made to a file.

        spdlog::get(Constants::Logging::DefaultLog)
            ->error(fmt::format("File no longer exists: {}", absolutePath.string()));

        return;
    }

    switch (event.eventType) {
        case FileEventType::Created: {
            OnFileCreation(event);
            break;
        }
        case FileEventType::Deleted: {
            OnFileDeletion(event);
            break;
        }
        case FileEventType::Touched: {
            OnFileModification(event);
            break;
        }
        case FileEventType::Renamed: {
            OnFileNameChange(event);
            break;
        }
        default: {
            std::abort();
        }
    }
}

void BaseModel::OnFileCreation(const FileEvent& event)
{
    auto* node =
        Utilities::FindNodeViaAbsolutePath(m_fileTree->GetRoot(), event.path.parent_path());

    if (!node) {
        return;
    }

    FileInfo fileInfo{ /* name = */ event.path.stem().wstring(),
                       /* extension = */ event.path.extension().wstring(),
                       /* size = */ event.fileSize,
                       /* type = */ FileType::Regular };

    node->AppendChild(VizBlock{ std::move(fileInfo) });
}

void BaseModel::OnFileDeletion(const FileEvent& event)
{
    auto* node = Utilities::FindNodeViaAbsolutePath(m_fileTree->GetRoot(), event.path);

    if (node) {
        node->DeleteFromTree();
        node = nullptr;
    }
}

void BaseModel::OnFileModification(const FileEvent& event)
{
    if (std::filesystem::is_regular_file(event.path)) {
        auto* node = Utilities::FindNodeViaAbsolutePath(m_fileTree->GetRoot(), event.path);

        if (node) {
            node->GetData().file.size = event.fileSize;
        }
    } else {
        // @todo What does it mean for a directory to be modified? Can this be ignored?
    }
}

void BaseModel::OnFileNameChange(const FileEvent& /*event*/)
{
    // @todo Need to associate new file names with old file names in order to resolve rename events.
}

void BaseModel::UpdateAncestorSizes(Tree<VizBlock>::Node* node)
{
    while (node) {
        auto* parent = node->GetParent();

        if (parent) {
            const auto totalSize = std::accumulate(
                Tree<VizBlock>::SiblingIterator{ parent->GetFirstChild() },
                Tree<VizBlock>::SiblingIterator{}, std::uintmax_t{ 0 },
                [](const auto runningTotal, const auto& node) noexcept {
                    Expects(node->file.size > 0);
                    return runningTotal + node->file.size;
                });

            Expects(totalSize > 0);
            parent->GetData().file.size = totalSize;
        }

        node = parent;
    }
}

bool BaseModel::IsFileSystemBeingMonitored() const
{
    return m_fileSystemObserver.IsActive();
}

std::optional<FileEvent> BaseModel::FetchNextVisualChange()
{
    FileEvent notification;

    const auto retrievedNotification = m_pendingVisualUpdates.TryPop(notification);
    if (!retrievedNotification) {
        return std::nullopt;
    }

    return notification;
}

std::optional<FileEvent> BaseModel::FetchNextModelChange()
{
    FileEvent notification;

    const auto retrievedNotification = m_pendingModelUpdates.TryPop(notification);
    if (!retrievedNotification) {
        return std::nullopt;
    }

    return notification;
}

std::filesystem::path BaseModel::GetRootPath() const
{
    return m_rootPath;
}

void BaseModel::SortNodes(Tree<VizBlock>& tree)
{
    for (auto& node : tree) {
        node.SortChildren([](const auto& lhs, const auto& rhs) noexcept {
            return lhs->file.size > rhs->file.size;
        });
    }
}
