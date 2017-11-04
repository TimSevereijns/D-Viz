#include "squarifiedTreemap.h"

#include "constants.h"

#include <algorithm>
#include <assert.h>
#include <limits>
#include <numeric>

#include <spdlog/spdlog.h>
#include <Stopwatch/Stopwatch.hpp>

namespace
{
   /**
    * @brief Ccomputes the total disk space represented by the nodes in the row.
    *
    * @param[in] row                The nodes in the whose size is to contribute to total row size.
    * @param[in] candidateItem      An optional additional item to be included in the row.
    *
    * @returns A total row size in bytes of disk space occupied.
    */
   auto ComputeBytesInRow(
      const std::vector<Tree<VizFile>::Node*>& row,
      const std::uintmax_t candidateSize)
   {
      auto sumOfFileSizes = std::accumulate(std::begin(row), std::end(row),
         std::uintmax_t{ 0 }, [] (const auto result, const auto* node)
      {
         return result + (*node)->file.size;
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
      const Block& land,
      const double percentageOfParent,
      VizFile& node,
      const size_t nodeCount)
   {
      const auto blockWidthPlusPadding = land.GetWidth() * percentageOfParent;
      const auto ratioBasedPadding = ((land.GetWidth() * 0.1) / nodeCount) / 2.0;

      auto widthPaddingPerSide = std::min(ratioBasedPadding, VisualizationModel::MAX_PADDING);
      auto finalBlockWidth = blockWidthPlusPadding - (2.0 * widthPaddingPerSide);
      if (finalBlockWidth < 0.0)
      {
         finalBlockWidth = blockWidthPlusPadding * VisualizationModel::PADDING_RATIO;
         widthPaddingPerSide = (blockWidthPlusPadding * (1.0 - VisualizationModel::PADDING_RATIO)) / 2.0;
      }

      const auto ratioBasedBlockDepth = std::abs(land.GetDepth() * VisualizationModel::PADDING_RATIO);
      const auto depthPaddingPerSide = std::min((land.GetDepth() - ratioBasedBlockDepth) / 2.0,
         VisualizationModel::MAX_PADDING);

      const auto finalBlockDepth = (depthPaddingPerSide == VisualizationModel::MAX_PADDING)
         ? std::abs(land.GetDepth()) - (2.0 * VisualizationModel::MAX_PADDING)
         : ratioBasedBlockDepth;

      const PrecisePoint offset
      {
         (land.GetWidth() * land.GetCoverage()) + widthPaddingPerSide,
         0.0,
         -depthPaddingPerSide
      };

      node.block = Block
      {
         land.GetOrigin() + offset,
         finalBlockWidth,
         VisualizationModel::BLOCK_HEIGHT,
         finalBlockDepth
      };

      const auto additionalCoverage = blockWidthPlusPadding / land.GetWidth();
      assert(additionalCoverage);

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
      const Block& land,
      const double percentageOfParent,
      VizFile& node,
      const size_t nodeCount)
   {
      const auto blockDepthPlusPadding = std::abs(land.GetDepth() * percentageOfParent);
      const auto ratioBasedPadding = (land.GetDepth() * 0.1) / nodeCount / 2.0;

      auto depthPaddingPerSide = std::min(ratioBasedPadding, VisualizationModel::MAX_PADDING);
      auto finalBlockDepth = blockDepthPlusPadding - (2.0 * depthPaddingPerSide);
      if (finalBlockDepth < 0)
      {
         finalBlockDepth = blockDepthPlusPadding * VisualizationModel::PADDING_RATIO;
         depthPaddingPerSide = (blockDepthPlusPadding * (1.0 - VisualizationModel::PADDING_RATIO)) / 2.0;
      }

      const auto ratioBasedWidth = land.GetWidth() * VisualizationModel::PADDING_RATIO;
      const auto widthPaddingPerSide = std::min((land.GetWidth() - ratioBasedWidth) / 2.0,
         VisualizationModel::MAX_PADDING);

      const auto finalBlockWidth = (widthPaddingPerSide == VisualizationModel::MAX_PADDING)
         ? land.GetWidth() - (2.0 * VisualizationModel::MAX_PADDING)
         : ratioBasedWidth;

      const PrecisePoint offset
      {
         widthPaddingPerSide,
         0.0,
         -(land.GetDepth() * land.GetCoverage()) - depthPaddingPerSide
      };

      node.block = Block
      {
         land.GetOrigin() + offset,
         finalBlockWidth,
         VisualizationModel::BLOCK_HEIGHT,
         std::abs(finalBlockDepth)
      };

      const auto additionalCoverage = blockDepthPlusPadding / land.GetDepth();
      assert(additionalCoverage);

      return additionalCoverage;
   }
}

SquarifiedTreeMap::SquarifiedTreeMap(const VisualizationParameters& parameters) :
   VisualizationModel{ parameters }
{
}

Block SquarifiedTreeMap::ComputeRemainingArea(const Block& block)
{
   const auto& originOfNextRow = block.GetNextRowOrigin();

   const PrecisePoint nearCorner
   {
      originOfNextRow.x(),
      originOfNextRow.y(),
      originOfNextRow.z()
   };

   const auto& originOfNextChild = block.ComputeNextChildOrigin();

   const PrecisePoint farCorner
   {
      originOfNextChild.x() + block.GetWidth(),
      originOfNextChild.y(),
      originOfNextChild.z() - block.GetDepth()
   };

   const Block remainingArea
   {
      /* origin = */ nearCorner,
      /* width = */ farCorner.x() - nearCorner.x(),
      /* height = */ VisualizationModel::BLOCK_HEIGHT,
      /* depth = */ farCorner.z() - nearCorner.z()
   };

   assert(remainingArea.HasVolume());
   return remainingArea;
}

double SquarifiedTreeMap::ComputeShortestEdgeOfRemainingBounds(const VizFile& node)
{
   const Block remainingRealEstate = ComputeRemainingArea(node.block);
   const auto shortestEdge = std::min(std::abs(remainingRealEstate.GetDepth()),
      std::abs(remainingRealEstate.GetWidth()));

   assert(shortestEdge > 0.0);
   return shortestEdge;
}

double SquarifiedTreeMap::ComputeWorstAspectRatio(
   const std::vector<Tree<VizFile>::Node*>& row,
   const uintmax_t candidateSize,
   VizFile& parentNode,
   const double shortestEdgeOfBounds)
{
   if (row.empty() && candidateSize == 0)
   {
      return std::numeric_limits<double>::max();
   }

   // Find the largest surface area if the row and candidate were laid out:

   std::uintmax_t largestNodeInBytes;
   if (!row.empty())
   {
      largestNodeInBytes = std::max(row.front()->GetData().file.size, candidateSize);
   }
   else if (candidateSize > 0)
   {
      largestNodeInBytes = candidateSize;
   }
   else
   {
      largestNodeInBytes = row.front()->GetData().file.size;
   }

   assert(largestNodeInBytes > 0);

   const auto updateOffset{ false };
   const std::uintmax_t bytesInRow = ComputeBytesInRow(row, candidateSize);
   const Block rowBounds = CalculateRowBounds(bytesInRow, parentNode, updateOffset);

   const auto totalRowArea = std::abs(rowBounds.GetWidth() * rowBounds.GetDepth());

   const auto largestArea =
      (static_cast<double>(largestNodeInBytes) / static_cast<double>(bytesInRow)) *
      totalRowArea;

   // Find the smallest surface area if the row and candidate were laid out:

   std::uintmax_t smallestNodeInBytes;
   if (candidateSize > 0 && !row.empty())
   {
      smallestNodeInBytes = std::min(row.back()->GetData().file.size, candidateSize);
   }
   else if (candidateSize > 0)
   {
      smallestNodeInBytes = candidateSize;
   }
   else
   {
      smallestNodeInBytes = row.back()->GetData().file.size;
   }

   assert(smallestNodeInBytes > 0);
   assert(totalRowArea > 0);

   const double smallestArea =
      (static_cast<double>(smallestNodeInBytes) / static_cast<double>(bytesInRow)) *
      totalRowArea;

   // Now compute the worst aspect ratio between the two choices above:

   const auto lengthSquared = shortestEdgeOfBounds * shortestEdgeOfBounds;
   const auto areaSquared = totalRowArea * totalRowArea;

   const auto worstRatio = std::max((lengthSquared * largestArea) / (areaSquared),
      (areaSquared) / (lengthSquared * smallestArea));

   assert(worstRatio > 0);
   return worstRatio;
}

void SquarifiedTreeMap::SquarifyAndLayoutRows(const std::vector<Tree<VizFile>::Node*>& nodes)
{
   if (nodes.empty())
   {
      return;
   }

   Tree<VizFile>::Node* parentNode = nodes.front()->GetParent();
   assert(parentNode);

   VizFile& parentVizNode = parentNode->GetData();
   assert(parentVizNode.block.HasVolume());

   std::vector<Tree<VizFile>::Node*> row;
   row.reserve(nodes.size());

   double shortestEdgeOfBounds = ComputeShortestEdgeOfRemainingBounds(parentVizNode);
   assert(shortestEdgeOfBounds > 0.0);

   for (Tree<VizFile>::Node* const node : nodes)
   {
      const double worstRatioWithNodeAddedToCurrentRow =
         ComputeWorstAspectRatio(row, node->GetData().file.size, parentVizNode,
         shortestEdgeOfBounds);

      const double worstRatioWithoutNodeAddedToCurrentRow =
         ComputeWorstAspectRatio(row, 0, parentVizNode, shortestEdgeOfBounds);

      assert(worstRatioWithNodeAddedToCurrentRow > 0.0);
      assert(worstRatioWithoutNodeAddedToCurrentRow > 0.0);

      if (worstRatioWithNodeAddedToCurrentRow <= worstRatioWithoutNodeAddedToCurrentRow)
      {
         row.emplace_back(node);
      }
      else
      {
         LayoutRow(row);

         row.clear();
         row.emplace_back(node);

         shortestEdgeOfBounds = ComputeShortestEdgeOfRemainingBounds(parentVizNode);
         assert(shortestEdgeOfBounds > 0.0);
      }
   }

   if (!row.empty())
   {
      LayoutRow(row);
   }
}

void SquarifiedTreeMap::SquarifyRecursively(const Tree<VizFile>::Node& root)
{
   Tree<VizFile>::Node* firstChild = root.GetFirstChild();
   if (!firstChild)
   {
      return;
   }

   std::vector<Tree<VizFile>::Node*> children;
   children.reserve(root.GetChildCount());
   children.emplace_back(firstChild);

   auto* nextChild = firstChild->GetNextSibling();
   while (nextChild)
   {
      children.emplace_back(nextChild);
      nextChild = nextChild->GetNextSibling();
   }

   SquarifyAndLayoutRows(children);

   for (auto* const child : children)
   {
      if (child)
      {
         SquarifyRecursively(*child);
      }
   }
}

Block SquarifiedTreeMap::CalculateRowBounds(
   std::uintmax_t bytesInRow,
   VizFile& parentNode,
   const bool updateOffset)
{
   const Block& parentBlock = parentNode.block;
   assert(parentBlock.HasVolume());

   Block remainingLand = ComputeRemainingArea(parentBlock);

   const double parentArea = parentBlock.GetWidth() * parentBlock.GetDepth();
   const double remainingArea = std::abs(remainingLand.GetWidth() * remainingLand.GetDepth());
   const double remainingBytes = (remainingArea / parentArea) * parentNode.file.size;

   const double rowToParentRatio = bytesInRow / remainingBytes;

   const auto& originOfNextRow = parentBlock.GetNextRowOrigin();

   const PrecisePoint nearCorner
   {
      originOfNextRow.x(),
      originOfNextRow.y(),
      originOfNextRow.z()
   };

   Block rowRealEstate;
   if (remainingLand.GetWidth() > std::abs(remainingLand.GetDepth()))
   {
      rowRealEstate = Block
      {
         nearCorner,
         remainingLand.GetWidth() * rowToParentRatio,
         remainingLand.GetHeight(),
         -remainingLand.GetDepth()
      };

      if (updateOffset)
      {
         const PrecisePoint nextRowOffset
         {
            rowRealEstate.GetWidth(),
            0.0,
            0.0
         };

         parentNode.block.SetNextRowOrigin(nearCorner + nextRowOffset);
      }
   }
   else
   {
      rowRealEstate = Block
      {
         PrecisePoint(nearCorner),
         remainingLand.GetWidth(),
         remainingLand.GetHeight(),
         -remainingLand.GetDepth() * rowToParentRatio
      };

      if (updateOffset)
      {
         const PrecisePoint nextRowOffset
         {
            0.0,
            0.0,
            -rowRealEstate.GetDepth()
         };

         parentNode.block.SetNextRowOrigin(nearCorner + nextRowOffset);
      }
   }

   assert(rowRealEstate.HasVolume());

   return rowRealEstate;
}

void SquarifiedTreeMap::LayoutRow(std::vector<Tree<VizFile>::Node*>& row)
{
   if (row.empty())
   {
      assert(!"Cannot layout an empty row.");
      return;
   }

   const std::uintmax_t bytesInRow = ComputeBytesInRow(row, /*candidateSize =*/ 0);

   Block land = CalculateRowBounds(bytesInRow, row.front()->GetParent()->GetData(),
      /*updateOffset =*/ true);

   assert(land.HasVolume());

   const auto nodeCount = row.size();

   double additionalCoverage = 0.0;

   for (auto* const node : row)
   {
      VizFile& data = node->GetData();

      const std::uintmax_t nodeFileSize = data.file.size;
      if (nodeFileSize == 0)
      {
         assert(!"Found a node without a file size!");
         return;
      }

      const double percentageOfParent =
         static_cast<double>(nodeFileSize) / static_cast<double>(bytesInRow);

      additionalCoverage = (land.GetWidth() > std::abs(land.GetDepth()))
         ? SlicePerpendicularToWidth(land, percentageOfParent, data, nodeCount)
         : SlicePerpendicularToDepth(land, percentageOfParent, data, nodeCount);

      assert(additionalCoverage > 0);
      assert(data.block.HasVolume());

      land.IncreaseCoverageBy(additionalCoverage);
   }
}

void SquarifiedTreeMap::Parse(const std::shared_ptr<Tree<VizFile>>& theTree)
{
   if (!theTree)
   {
      assert(!"Whoops, no tree in sight!");
      return;
   }

   m_theTree = theTree;

   Stopwatch<std::chrono::milliseconds>([&]
   {
      VisualizationModel::SortNodes(*m_theTree);
   }, [] (const auto& elapsed, const auto& units) noexcept
   {
      spdlog::get(Constants::Logging::DEFAULT_LOG)->info(
         fmt::format("Sorted tree in: {} {}", elapsed.count(), units));
   });

   const Block rootBlock
   {
      PrecisePoint{ },
      VisualizationModel::ROOT_BLOCK_WIDTH,
      VisualizationModel::BLOCK_HEIGHT,
      VisualizationModel::ROOT_BLOCK_DEPTH
   };

   theTree->GetRoot()->GetData().block = rootBlock;

   Stopwatch<std::chrono::milliseconds>([&]
   {
      SquarifyRecursively(*theTree->GetRoot());
   }, [] (const auto& elapsed, const auto& units) noexcept
   {
      spdlog::get(Constants::Logging::DEFAULT_LOG)->info(
         fmt::format("Visualization Generated in: {} {}", elapsed.count(), units));
   });

   m_hasDataBeenParsed = true;
}
