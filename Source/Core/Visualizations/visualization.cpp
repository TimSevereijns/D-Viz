#include "Visualizations/visualization.h"
#include "Scanner/Monitor/fileChangeNotification.hpp"
#include "Scanner/scanningUtilities.h"
#include "Utilities/utilities.hpp"
#include "Visualizations/ray.h"
#include "constants.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/optional.hpp>

#include <Stopwatch/Stopwatch.hpp>
#include <gsl/gsl_assert>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>

#include <QColor>
#include <QRectF>

namespace
{
    constexpr QVector3D POSITIVE_X_NORMAL{ 1.0f, 0.0f, 0.0f };
    constexpr QVector3D POSITIVE_Y_NORMAL{ 0.0f, 1.0f, 0.0f };
    constexpr QVector3D POSITIVE_Z_NORMAL{ 0.0f, 0.0f, 1.0f };
    constexpr QVector3D NEGATIVE_X_NORMAL{ -1.0f, 0.0f, 0.0f };
    constexpr QVector3D NEGATIVE_Y_NORMAL{ 0.0f, -1.0f, 0.0f };
    constexpr QVector3D NEGATIVE_Z_NORMAL{ 0.0f, 0.0f, -1.0f };

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
    boost::optional<QVector3D> DoesRayIntersectPlane(
        const Ray& ray, const QVector3D& pointOnPlane, const QVector3D& planeNormal)
    {
        constexpr auto epsilon{ 0.0001f };

        const auto denominator = QVector3D::dotProduct(ray.Direction(), planeNormal);
        if (std::abs(denominator) < epsilon) {
            return boost::none;
        }

        const auto numerator = QVector3D::dotProduct(pointOnPlane - ray.Origin(), planeNormal);

        const auto scalar = numerator / denominator;
        const bool doesRayHitPlane = std::abs(scalar) > epsilon;

        if (!doesRayHitPlane) {
            return boost::none;
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
    boost::optional<QVector3D>
    FindClosestIntersectionPoint(const Ray& ray, const std::vector<QVector3D>& allIntersections)
    {
        const auto& closest = std::min_element(
            std::begin(allIntersections),
            std::end(allIntersections), [&ray](const auto& lhs, const auto& rhs) noexcept {
                return (ray.Origin().distanceToPoint(lhs) < ray.Origin().distanceToPoint(rhs));
            });

        if (closest == std::end(allIntersections)) {
            return boost::none;
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
    boost::optional<QVector3D> DoesRayIntersectBlock(const Ray& ray, const Block& block)
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

            const boost::optional<QVector3D> intersectionPoint =
                DoesRayIntersectPlane(ray, pointOnPlane, POSITIVE_Y_NORMAL);

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

            const boost::optional<QVector3D> intersectionPoint =
                DoesRayIntersectPlane(ray, pointOnPlane, POSITIVE_Z_NORMAL);

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

            const boost::optional<QVector3D> intersectionPoint =
                DoesRayIntersectPlane(ray, pointOnPlane, NEGATIVE_Z_NORMAL);

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

            const boost::optional<QVector3D> intersectionPoint =
                DoesRayIntersectPlane(ray, pointOnPlane, NEGATIVE_X_NORMAL);

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

            const boost::optional<QVector3D> intersectionPoint =
                DoesRayIntersectPlane(ray, pointOnPlane, POSITIVE_X_NORMAL);

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

        const auto notTheRightFileType =
            parameters.onlyShowDirectories && node->GetData().file.type != FileType::DIRECTORY;

        while (node) {
            if (node->GetData().file.size < parameters.minimumFileSize || notTheRightFileType) {
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
} // namespace

VisualizationModel::VisualizationModel(
    std::unique_ptr<FileMonitorBase> fileMonitor, const std::experimental::filesystem::path& path)
    : m_rootPath{ path }, m_fileSystemObserver{ std::move(fileMonitor), path }
{
}

void VisualizationModel::UpdateBoundingBoxes()
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

Tree<VizBlock>::Node* VisualizationModel::FindNearestIntersection(
    const Camera& camera, const Ray& ray, const Settings::VisualizationParameters& parameters) const
{
    if (!m_hasDataBeenParsed) {
        return nullptr;
    }

    Tree<VizBlock>::Node* nearestIntersection = nullptr;

    Stopwatch<std::chrono::microseconds>(
        [&]() noexcept {
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
        },
        [](const auto& elapsed, const auto& units) noexcept {
            spdlog::get(Constants::Logging::DEFAULT_LOG)
                ->info(fmt::format("Selected node in: {} {}", elapsed.count(), units));
        });

    return nearestIntersection;
}

Tree<VizBlock>& VisualizationModel::GetTree()
{
    Expects(m_fileTree != nullptr);

    return *m_fileTree;
}

const Tree<VizBlock>& VisualizationModel::GetTree() const
{
    Expects(m_fileTree != nullptr);

    return *m_fileTree;
}

const std::vector<const Tree<VizBlock>::Node*>& VisualizationModel::GetHighlightedNodes() const
{
    return m_highlightedNodes;
}

std::vector<const Tree<VizBlock>::Node*>& VisualizationModel::GetHighlightedNodes()
{
    return m_highlightedNodes;
}

void VisualizationModel::ClearHighlightedNodes()
{
    if (m_highlightedNodes.size() == 0) {
        return;
    }

    m_highlightedNodes.clear();
}

void VisualizationModel::SelectNode(const Tree<VizBlock>::Node& node)
{
    m_selectedNode = &node;
}

const Tree<VizBlock>::Node* VisualizationModel::GetSelectedNode()
{
    return m_selectedNode;
}

void VisualizationModel::ClearSelectedNode()
{
    m_selectedNode = nullptr;
}

TreemapMetadata VisualizationModel::GetTreemapMetadata()
{
    return m_metadata;
}

void VisualizationModel::SetTreemapMetadata(TreemapMetadata&& data)
{
    m_metadata = data;
}

void VisualizationModel::HighlightAncestors(const Tree<VizBlock>::Node& node)
{
    auto* currentNode = node.GetParent();
    while (currentNode) {
        m_highlightedNodes.emplace_back(currentNode);
        currentNode = currentNode->GetParent();
    }
}

void VisualizationModel::HighlightDescendants(
    const Tree<VizBlock>::Node& node, const Settings::VisualizationParameters& parameters)
{
    std::for_each(
        Tree<VizBlock>::LeafIterator{ &node }, Tree<VizBlock>::LeafIterator{},
        [&](const auto& node) {
            if ((parameters.onlyShowDirectories && node->file.type != FileType::DIRECTORY) ||
                node->file.size < parameters.minimumFileSize) {
                return;
            }

            m_highlightedNodes.emplace_back(&node);
        });
}

void VisualizationModel::HighlightMatchingFileExtension(
    const Tree<VizBlock>::Node& sampleNode, const Settings::VisualizationParameters& parameters)
{
    std::for_each(
        Tree<VizBlock>::LeafIterator{ GetTree().GetRoot() }, Tree<VizBlock>::LeafIterator{},
        [&](const auto& node) {
            if ((parameters.onlyShowDirectories && node->file.type != FileType::DIRECTORY) ||
                node->file.size < parameters.minimumFileSize ||
                node->file.extension != sampleNode->file.extension) {
                return;
            }

            m_highlightedNodes.emplace_back(&node);
        });
}

void VisualizationModel::HighlightMatchingFileName(
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
                (!shouldSearchDirectories && file.type == FileType::DIRECTORY) ||
                (!shouldSearchFiles && file.type == FileType::REGULAR)) {
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

            m_highlightedNodes.emplace_back(&node);
        });
}

void VisualizationModel::StartMonitoringFileSystem()
{
    Expects(m_fileTree != nullptr);

    m_fileSystemObserver.StartMonitoring(m_fileTree->GetRoot());
}

void VisualizationModel::StopMonitoringFileSystem()
{
    m_fileSystemObserver.StopMonitoring();
}

void VisualizationModel::WaitForNextChange()
{
    m_fileSystemObserver.WaitForNextChange();
}

void VisualizationModel::RefreshTreemap()
{
    auto notification = m_fileSystemObserver.FetchNextChange();
    while (notification) {
        UpdateAffectedNodes(*notification);

        notification = m_fileSystemObserver.FetchNextChange();
    }

    // @todo Sort the tree.
    // @todo Update all sizes.
}

void VisualizationModel::UpdateAffectedNodes(const FileChangeNotification& notification)
{
    const auto absolutePath =
        std::experimental::filesystem::absolute(notification.relativePath, m_rootPath);

    std::error_code errorCode;

    if (notification.status != FileModification::DELETED &&
        !std::experimental::filesystem::exists(absolutePath) && !errorCode) {
        // @note The absence of a file may not necessarily indicate a bug, since there tend to be
        // a lot of transient files that may only exist for a fraction of a second. For example,
        // some applications tend to create temporary files when saving changes made to a file.

        spdlog::get(Constants::Logging::DEFAULT_LOG)
            ->error(fmt::format("File no longer exists: {}", absolutePath.string()));

        return;
    }

    switch (notification.status) {
        case FileModification::CREATED: {
            OnFileCreation(notification);
            break;
        }
        case FileModification::DELETED: {
            OnFileDeletion(notification);
            break;
        }
        case FileModification::TOUCHED: {
            OnFileModification(notification);
            break;
        }
        case FileModification::RENAMED: {
            OnFileNameChange(notification);
            break;
        }
        default: {
            std::abort();
        }
    }
}

void VisualizationModel::OnFileCreation(const FileChangeNotification& notification)
{
    // @todo Find parent node from path:
    auto* parentNode =
        Utilities::FindNodeUsingRelativePath(m_fileTree->GetRoot(), notification.relativePath);

    const auto absolutePath =
        std::experimental::filesystem::absolute(notification.relativePath, m_rootPath);

    if (std::experimental::filesystem::is_directory(absolutePath)) //< @todo Check symlink status...
    {
        FileInfo directoryInfo{ /* name = */ notification.relativePath.filename().wstring(),
                                /* extension = */ L"",
                                /* size = */ 0,
                                /* type = */ FileType::DIRECTORY };

        parentNode->AppendChild(VizBlock{ std::move(directoryInfo) });
    } else {
        const auto fileSize = Scanner::ComputeFileSize(absolutePath);

        FileInfo fileInfo{
            /* name = */ notification.relativePath.filename().stem().wstring(),
            /* extension = */ notification.relativePath.filename().extension().wstring(),
            /* size = */ fileSize,
            /* type = */ FileType::REGULAR
        };

        parentNode->AppendChild(VizBlock{ std::move(fileInfo) });
    }
}

void VisualizationModel::OnFileDeletion(const FileChangeNotification& notification)
{
    auto* node =
        Utilities::FindNodeUsingRelativePath(m_fileTree->GetRoot(), notification.relativePath);

    if (node) {
        node->DeleteFromTree();
        node = nullptr;
    }
}

void VisualizationModel::OnFileModification(const FileChangeNotification& notification)
{
    const auto absolutePath =
        std::experimental::filesystem::absolute(notification.relativePath, m_rootPath);

    if (std::experimental::filesystem::is_directory(absolutePath)) {
        // @todo What does it mean for a directory to be modified?
    } else {
        const auto fileSize = Scanner::ComputeFileSize(absolutePath);

        auto* node =
            Utilities::FindNodeUsingRelativePath(m_fileTree->GetRoot(), notification.relativePath);

        if (node) {
            node->GetData().file.size = fileSize;
        }
    }
}

void VisualizationModel::OnFileNameChange(const FileChangeNotification& /*notification*/)
{
    // @todo Need to associate new file names with old file names in order to resolve rename events.
}

void VisualizationModel::UpdateAncestorSizes(Tree<VizBlock>::Node* node)
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

bool VisualizationModel::IsFileSystemBeingMonitored() const
{
    return m_fileSystemObserver.IsActive();
}

boost::optional<FileChangeNotification> VisualizationModel::FetchNextFileSystemChange()
{
    return m_fileSystemObserver.FetchNextChange();
}

std::experimental::filesystem::path VisualizationModel::GetRootPath() const
{
    return m_rootPath;
}

void VisualizationModel::SortNodes(Tree<VizBlock>& tree)
{
    for (auto& node : tree) {
        node.SortChildren([](const auto& lhs, const auto& rhs) noexcept {
            return lhs->file.size > rhs->file.size;
        });
    }
}
