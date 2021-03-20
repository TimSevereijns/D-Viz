#include "Model/squarifiedTreemap.h"
#include "constants.h"

#include <algorithm>
#include <limits>
#include <numeric>

#include <gsl/gsl_assert>
#include <spdlog/spdlog.h>
#include <stopwatch.h>

namespace
{
    /**
     * @brief Computes the total disk space represented by the nodes in the row.
     *
     * @param[in] row                The nodes in the whose size is to contribute to total row size.
     * @param[in] candidateItem      An optional additional item to be included in the row.
     *
     * @returns A total row size in bytes of disk space occupied.
     */
    std::uintmax_t ComputeBytesInRow(
        const std::vector<Tree<VizBlock>::Node*>& row, const std::uintmax_t candidateSize)
    {
        auto sumOfFileSizes = std::accumulate(std::begin(row), std::end(row), std::uintmax_t{ 0 }, [
        ](const auto runningTotal, const auto* node) noexcept {
            return runningTotal + (*node)->file.size;
        });

        sumOfFileSizes += candidateSize;

        return sumOfFileSizes;
    }

    /**
     * @brief Slice perpendicular to block width.
     *
     * @param[in] land               The node, or "land," to lay the current node out upon.
     * @param[in] percentageOfParent The percentage of the parent node that the current node will
     *                               consume.
     * @param[in, out] node          The node to be laid out upon the land.
     * @param[in] nodeCount          The number of sibling nodes that the node has in its row.
     *
     * @returns The additional coverage, as a percentage, of total parent area.
     */
    double SlicePerpendicularToWidth(
        const Block& land, const double percentageOfParent, VizBlock& node, const size_t nodeCount)
    {
        using namespace Constants;

        const auto availableDepth = land.GetDepth();
        const auto availableWidth = land.GetWidth();

        const auto blockWidthPlusPadding = availableWidth * percentageOfParent;
        const auto ratioBasedPadding = ((availableWidth * 0.1) / nodeCount) / 2.0;

        const auto widthPadding = std::min(ratioBasedPadding, Treemap::MaxPadding);
        const auto finalBlockWidth = blockWidthPlusPadding - (2.0 * widthPadding);

        if (finalBlockWidth < 0.0) {
            assert(false);
        }

        const auto blockDepth = std::abs(availableDepth * Treemap::PaddingRatio);
        const auto depthPadding =
            std::min((availableDepth - blockDepth) / 2.0, Treemap::MaxPadding);

        const auto finalBlockDepth = depthPadding == Treemap::MaxPadding
                                         ? std::abs(availableDepth) - (2.0 * Treemap::MaxPadding)
                                         : blockDepth;

        const auto x = (availableWidth * land.GetCoverage()) + widthPadding;
        const auto y = 0.0;
        const auto z = -depthPadding;

        const PrecisePoint blockOrigin = land.GetOrigin() + PrecisePoint{ x, y, z };
        node.block = Block{ blockOrigin, finalBlockWidth, Treemap::BlockHeight, finalBlockDepth };

        const auto additionalCoverage = blockWidthPlusPadding / availableWidth;
        Expects(additionalCoverage > 0.0);

        return additionalCoverage;
    }

    /**
     * @brief Slice perpendicular to block depth.
     *
     * @param[in] land               The node, or "land," to lay the current node out upon.
     * @param[in] percentageOfParent The percentage of the parent node that the current node will
     *                               consume.
     * @param[in, out] node          The node to be laid out upon the land.
     * @param[in] nodeCount          The number of sibling nodes that the node has in its row.
     *
     * @return The additional coverage, as a percentage, of total parent area.
     */
    double SlicePerpendicularToDepth(
        const Block& land, const double percentageOfParent, VizBlock& node, const size_t nodeCount)
    {
        using namespace Constants;

        const auto availableDepth = land.GetDepth();
        const auto availableWidth = land.GetWidth();

        const auto blockDepthPlusPadding = std::abs(availableDepth * percentageOfParent);
        const auto ratioBasedPadding = (availableDepth * 0.1) / nodeCount / 2.0;

        const auto depthPadding = std::min(ratioBasedPadding, Treemap::MaxPadding);
        const auto finalBlockDepth = blockDepthPlusPadding - (2.0 * depthPadding);

        if (finalBlockDepth < 0) {
            assert(false);
        }

        const auto blockWidth = availableWidth * Treemap::PaddingRatio;
        const auto widthPadding =
            std::min((availableWidth - blockWidth) / 2.0, Treemap::MaxPadding);

        const auto finalBlockWidth = widthPadding == Treemap::MaxPadding
                                         ? availableWidth - (2.0 * Treemap::MaxPadding)
                                         : blockWidth;

        const auto x = widthPadding;
        const auto y = 0.0;
        const auto z = -(availableDepth * land.GetCoverage()) - depthPadding;

        const PrecisePoint blockOrigin = land.GetOrigin() + PrecisePoint{ x, y, z };
        node.block = Block{ blockOrigin, finalBlockWidth, Treemap::BlockHeight, finalBlockDepth };

        const auto additionalCoverage = blockDepthPlusPadding / availableDepth;
        Expects(additionalCoverage > 0.0);

        return additionalCoverage;
    }
} // namespace

Block SquarifiedTreeMap::ComputeRemainingArea(const Block& block)
{
    const auto& originOfNextRow = block.GetNextRowOrigin();
    const auto& originOfNextChild = block.ComputeNextChildOrigin();

    const PrecisePoint nearCorner{ originOfNextRow.x(), originOfNextRow.y(), originOfNextRow.z() };
    const PrecisePoint farCorner{ originOfNextChild.x() + block.GetWidth(), originOfNextChild.y(),
                                  originOfNextChild.z() - block.GetDepth() };

    const Block remainingArea{ /* origin = */ nearCorner,
                               /* width = */ farCorner.x() - nearCorner.x(),
                               /* height = */ Constants::Treemap::BlockHeight,
                               /* depth = */ farCorner.z() - nearCorner.z() };

    Expects(remainingArea.HasVolume());
    return remainingArea;
}

double SquarifiedTreeMap::ComputeShortestEdgeOfRemainingBounds(const VizBlock& node)
{
    const auto remainingRealEstate = ComputeRemainingArea(node.block);
    const auto shortestEdge = std::min(
        std::abs(remainingRealEstate.GetDepth()), std::abs(remainingRealEstate.GetWidth()));

    Expects(shortestEdge > 0.0);
    return shortestEdge;
}

double SquarifiedTreeMap::ComputeWorstAspectRatio(
    const std::vector<Tree<VizBlock>::Node*>& row, const uintmax_t candidateSize,
    VizBlock& parentNode, const double shortestEdgeOfBounds)
{
    if (row.empty() && candidateSize == 0) {
        return std::numeric_limits<double>::max();
    }

    // Find the largest surface area if the row and candidate were laid out:

    std::uintmax_t largestNodeInBytes;
    if (!row.empty()) {
        largestNodeInBytes = std::max(row.front()->GetData().file.size, candidateSize);
    } else if (candidateSize > 0) {
        largestNodeInBytes = candidateSize;
    } else {
        largestNodeInBytes = row.front()->GetData().file.size;
    }

    Expects(largestNodeInBytes > 0);

    constexpr auto updateOffset = false;
    const auto bytesInRow = ComputeBytesInRow(row, candidateSize);
    const auto rowBounds = CalculateRowBounds(bytesInRow, parentNode, updateOffset);
    const auto totalRowArea = std::abs(rowBounds.GetWidth() * rowBounds.GetDepth());

    const auto largestArea = largestNodeInBytes / static_cast<double>(bytesInRow) * totalRowArea;

    // Find the smallest surface area if the row and candidate were laid out:

    std::uintmax_t smallestNodeInBytes;
    if (candidateSize > 0 && !row.empty()) {
        smallestNodeInBytes = std::min(row.back()->GetData().file.size, candidateSize);
    } else if (candidateSize > 0) {
        smallestNodeInBytes = candidateSize;
    } else {
        smallestNodeInBytes = row.back()->GetData().file.size;
    }

    Expects(smallestNodeInBytes > 0);
    Expects(totalRowArea > 0);

    const auto smallestArea = smallestNodeInBytes / static_cast<double>(bytesInRow) * totalRowArea;

    // Now compute the worst aspect ratio between the two choices above:

    const auto lengthSquared = shortestEdgeOfBounds * shortestEdgeOfBounds;
    const auto areaSquared = totalRowArea * totalRowArea;

    const auto worstRatio = std::max(
        (lengthSquared * largestArea) / areaSquared, areaSquared / (lengthSquared * smallestArea));

    Expects(worstRatio > 0);
    return worstRatio;
}

void SquarifiedTreeMap::SquarifyAndLayoutRows(const std::vector<Tree<VizBlock>::Node*>& nodes)
{
    if (nodes.empty()) {
        return;
    }

    Tree<VizBlock>::Node* parentNode = nodes.front()->GetParent();
    Expects(parentNode);

    VizBlock& parentVizNode = parentNode->GetData();
    Expects(parentVizNode.block.HasVolume());

    std::vector<Tree<VizBlock>::Node*> row;
    row.reserve(nodes.size());

    double shortestEdgeOfBounds = ComputeShortestEdgeOfRemainingBounds(parentVizNode);
    Expects(shortestEdgeOfBounds > 0.0);

    for (Tree<VizBlock>::Node* const node : nodes) {
        const double worstRatioWithNodeAddedToCurrentRow = ComputeWorstAspectRatio(
            row, node->GetData().file.size, parentVizNode, shortestEdgeOfBounds);

        const double worstRatioWithoutNodeAddedToCurrentRow =
            ComputeWorstAspectRatio(row, 0, parentVizNode, shortestEdgeOfBounds);

        Expects(worstRatioWithNodeAddedToCurrentRow > 0.0);
        Expects(worstRatioWithoutNodeAddedToCurrentRow > 0.0);

        if (worstRatioWithNodeAddedToCurrentRow <= worstRatioWithoutNodeAddedToCurrentRow) {
            row.emplace_back(node);
        } else {
            LayoutRow(row);

            row.clear();
            row.emplace_back(node);

            shortestEdgeOfBounds = ComputeShortestEdgeOfRemainingBounds(parentVizNode);
            Expects(shortestEdgeOfBounds > 0.0);
        }
    }

    if (!row.empty()) {
        LayoutRow(row);
    }
}

void SquarifiedTreeMap::SquarifyRecursively(const Tree<VizBlock>::Node& root)
{
    Tree<VizBlock>::Node* firstChild = root.GetFirstChild();
    if (!firstChild) {
        return;
    }

    std::vector<Tree<VizBlock>::Node*> children;
    children.reserve(root.GetChildCount());
    children.emplace_back(firstChild);

    auto* nextChild = firstChild->GetNextSibling();
    while (nextChild) {
        children.emplace_back(nextChild);
        nextChild = nextChild->GetNextSibling();
    }

    SquarifyAndLayoutRows(children);

    for (auto* const child : children) {
        if (child) {
            SquarifyRecursively(*child);
        }
    }
}

Block SquarifiedTreeMap::CalculateRowBounds(
    std::uintmax_t bytesInRow, VizBlock& parentNode, const bool updateOffset)
{
    const Block& parentBlock = parentNode.block;
    Expects(parentBlock.HasVolume());

    const Block remainingLand = ComputeRemainingArea(parentBlock);

    const double parentArea = parentBlock.GetWidth() * parentBlock.GetDepth();
    const double remainingArea = std::abs(remainingLand.GetWidth() * remainingLand.GetDepth());
    const double remainingBytes = (remainingArea / parentArea) * parentNode.file.size;
    const double rowToParentRatio = bytesInRow / remainingBytes;

    const PrecisePoint& originOfNextRow = parentBlock.GetNextRowOrigin();
    const PrecisePoint nearCorner{ originOfNextRow.x(), originOfNextRow.y(), originOfNextRow.z() };

    Block rowRealEstate;
    if (remainingLand.GetWidth() > std::abs(remainingLand.GetDepth())) {
        rowRealEstate = Block{ nearCorner, remainingLand.GetWidth() * rowToParentRatio,
                               remainingLand.GetHeight(), -remainingLand.GetDepth() };

        if (updateOffset) {
            const PrecisePoint nextRowOffset{ rowRealEstate.GetWidth(), 0.0, 0.0 };
            parentNode.block.SetNextRowOrigin(nearCorner + nextRowOffset);
        }
    } else {
        rowRealEstate =
            Block{ PrecisePoint(nearCorner), remainingLand.GetWidth(), remainingLand.GetHeight(),
                   -remainingLand.GetDepth() * rowToParentRatio };

        if (updateOffset) {
            const PrecisePoint nextRowOffset{ 0.0, 0.0, -rowRealEstate.GetDepth() };
            parentNode.block.SetNextRowOrigin(nearCorner + nextRowOffset);
        }
    }

    Expects(rowRealEstate.HasVolume());

    return rowRealEstate;
}

void SquarifiedTreeMap::LayoutRow(std::vector<Tree<VizBlock>::Node*>& row)
{
    if (row.empty()) {
        Expects(!"Cannot layout an empty row.");
        return;
    }

    const std::uintmax_t bytesInRow = ComputeBytesInRow(row, /*candidateSize =*/0);

    constexpr auto updateOffset = true;
    Block land = CalculateRowBounds(bytesInRow, row.front()->GetParent()->GetData(), updateOffset);
    Expects(land.HasVolume());

    const auto nodeCount = row.size();

    for (auto* const node : row) {
        VizBlock& data = node->GetData();

        const std::uintmax_t nodeFileSize = data.file.size;
        if (nodeFileSize == 0) {
            Expects(!"Found a node without a file size!");
            return;
        }

        const auto percentageOfParent =
            static_cast<double>(nodeFileSize) / static_cast<double>(bytesInRow);

        const auto additionalCoverage =
            land.GetWidth() > std::abs(land.GetDepth())
                ? SlicePerpendicularToWidth(land, percentageOfParent, data, nodeCount)
                : SlicePerpendicularToDepth(land, percentageOfParent, data, nodeCount);

        Expects(additionalCoverage > 0);
        Expects(data.block.HasVolume());

        land.IncreaseCoverageBy(additionalCoverage);
    }
}

SquarifiedTreeMap::SquarifiedTreeMap(
    std::unique_ptr<FileMonitorBase> fileMonitor, const std::filesystem::path& path)
    : BaseModel{ std::move(fileMonitor), path }
{
}

void SquarifiedTreeMap::Parse(const std::shared_ptr<Tree<VizBlock>>& theTree)
{
    if (!theTree) {
        Expects(!"Whoops, no tree in sight!");
        return;
    }

    m_fileTree = theTree;

    const auto sortingStopwatch =
        Stopwatch<std::chrono::milliseconds>([&]() { BaseModel::SortNodes(*m_fileTree); });

    const auto& log = spdlog::get(Constants::Logging::DefaultLog);

    log->info(fmt::format(
        "Sorted tree in: {:n} {}", sortingStopwatch.GetElapsedTime().count(),
        sortingStopwatch.GetUnitsAsString()));

    m_fileTree->GetRoot()->GetData().block =
        Block{ PrecisePoint{}, Constants::Treemap::RootBlockWidth, Constants::Treemap::BlockHeight,
               Constants::Treemap::RootBlockDepth };

    const auto squarificationStopwatch = Stopwatch<std::chrono::milliseconds>(
        [&]() { SquarifyRecursively(*m_fileTree->GetRoot()); });

    log->info(fmt::format(
        "Visualization Generated in: {:n} {}", squarificationStopwatch.GetElapsedTime().count(),
        squarificationStopwatch.GetUnitsAsString()));

    m_hasDataBeenParsed = true;
}
