#include "squarifiedTreemap.h"

#include "constants.h"

#include <algorithm>
#include <limits>
#include <numeric>

#include <gsl/gsl_assert>
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
   std::uintmax_t ComputeBytesInRow(
      const std::vector<Tree<VizBlock>::Node*>& row,
      const std::uintmax_t candidateSize)
   {
      auto sumOfFileSizes = std::accumulate(std::begin(row), std::end(row),
         std::uintmax_t{ 0 }, [] (const auto runningTotal, const auto* node) noexcept
      {
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
      const Block& land,
      const double percentageOfParent,
      VizBlock& node,
      const size_t nodeCount)
   {
      using namespace Constants;

      const auto blockWidthPlusPadding = land.GetWidth() * percentageOfParent;
      const auto ratioBasedPadding = ((land.GetWidth() * 0.1) / nodeCount) / 2.0;

      auto widthPaddingPerSide = std::min(ratioBasedPadding, Visualization::MAX_PADDING);
      auto finalBlockWidth = blockWidthPlusPadding - (2.0 * widthPaddingPerSide);
      if (finalBlockWidth < 0.0)
      {
         finalBlockWidth = blockWidthPlusPadding * Visualization::PADDING_RATIO;
         widthPaddingPerSide = (blockWidthPlusPadding * (1.0 - Visualization::PADDING_RATIO)) / 2.0;
      }

      const auto ratioBasedBlockDepth = std::abs(land.GetDepth() * Visualization::PADDING_RATIO);
      const auto depthPaddingPerSide = std::min((land.GetDepth() - ratioBasedBlockDepth) / 2.0,
         Visualization::MAX_PADDING);

      const auto finalBlockDepth = (depthPaddingPerSide == Visualization::MAX_PADDING)
         ? std::abs(land.GetDepth()) - (2.0 * Visualization::MAX_PADDING)
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
         Visualization::BLOCK_HEIGHT,
         finalBlockDepth
      };

      const auto additionalCoverage = blockWidthPlusPadding / land.GetWidth();
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
      const Block& land,
      const double percentageOfParent,
      VizBlock& node,
      const size_t nodeCount)
   {
      using namespace Constants;

      const auto blockDepthPlusPadding = std::abs(land.GetDepth() * percentageOfParent);
      const auto ratioBasedPadding = (land.GetDepth() * 0.1) / nodeCount / 2.0;

      auto depthPaddingPerSide = std::min(ratioBasedPadding, Visualization::MAX_PADDING);
      auto finalBlockDepth = blockDepthPlusPadding - (2.0 * depthPaddingPerSide);
      if (finalBlockDepth < 0)
      {
         finalBlockDepth = blockDepthPlusPadding * Visualization::PADDING_RATIO;
         depthPaddingPerSide = (blockDepthPlusPadding * (1.0 - Visualization::PADDING_RATIO)) / 2.0;
      }

      const auto ratioBasedWidth = land.GetWidth() * Visualization::PADDING_RATIO;
      const auto widthPaddingPerSide = std::min((land.GetWidth() - ratioBasedWidth) / 2.0,
         Visualization::MAX_PADDING);

      const auto finalBlockWidth = (widthPaddingPerSide == Visualization::MAX_PADDING)
         ? land.GetWidth() - (2.0 * Visualization::MAX_PADDING)
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
         Visualization::BLOCK_HEIGHT,
         std::abs(finalBlockDepth)
      };

      const auto additionalCoverage = blockDepthPlusPadding / land.GetDepth();
      Expects(additionalCoverage);

      return additionalCoverage;
   }
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
      /* height = */ Constants::Visualization::BLOCK_HEIGHT,
      /* depth = */ farCorner.z() - nearCorner.z()
   };

   Expects(remainingArea.HasVolume());
   return remainingArea;
}

double SquarifiedTreeMap::ComputeShortestEdgeOfRemainingBounds(const VizBlock& node)
{
   const Block remainingRealEstate = ComputeRemainingArea(node.block);
   const auto shortestEdge = std::min(std::abs(remainingRealEstate.GetDepth()),
      std::abs(remainingRealEstate.GetWidth()));

   assert(shortestEdge > 0.0);
   return shortestEdge;
}

double SquarifiedTreeMap::ComputeWorstAspectRatio(
   const std::vector<Tree<VizBlock>::Node*>& row,
   const uintmax_t candidateSize,
   VizBlock& parentNode,
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

   Expects(largestNodeInBytes > 0);

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

   Expects(smallestNodeInBytes > 0);
   Expects(totalRowArea > 0);

   const double smallestArea =
      (static_cast<double>(smallestNodeInBytes) / static_cast<double>(bytesInRow)) *
      totalRowArea;

   // Now compute the worst aspect ratio between the two choices above:

   const auto lengthSquared = shortestEdgeOfBounds * shortestEdgeOfBounds;
   const auto areaSquared = totalRowArea * totalRowArea;

   const auto worstRatio = std::max((lengthSquared * largestArea) / (areaSquared),
      (areaSquared) / (lengthSquared * smallestArea));

   Expects(worstRatio > 0);
   return worstRatio;
}

void SquarifiedTreeMap::SquarifyAndLayoutRows(const std::vector<Tree<VizBlock>::Node*>& nodes)
{
   if (nodes.empty())
   {
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

   for (Tree<VizBlock>::Node* const node : nodes)
   {
      const double worstRatioWithNodeAddedToCurrentRow =
         ComputeWorstAspectRatio(row, node->GetData().file.size, parentVizNode,
         shortestEdgeOfBounds);

      const double worstRatioWithoutNodeAddedToCurrentRow =
         ComputeWorstAspectRatio(row, 0, parentVizNode, shortestEdgeOfBounds);

      Expects(worstRatioWithNodeAddedToCurrentRow > 0.0);
      Expects(worstRatioWithoutNodeAddedToCurrentRow > 0.0);

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
         Expects(shortestEdgeOfBounds > 0.0);
      }
   }

   if (!row.empty())
   {
      LayoutRow(row);
   }
}

void SquarifiedTreeMap::SquarifyRecursively(const Tree<VizBlock>::Node& root)
{
   Tree<VizBlock>::Node* firstChild = root.GetFirstChild();
   if (!firstChild)
   {
      return;
   }

   std::vector<Tree<VizBlock>::Node*> children;
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
   VizBlock& parentNode,
   const bool updateOffset)
{
   const Block& parentBlock = parentNode.block;
   Expects(parentBlock.HasVolume());

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

   Expects(rowRealEstate.HasVolume());

   return rowRealEstate;
}

void SquarifiedTreeMap::LayoutRow(std::vector<Tree<VizBlock>::Node*>& row)
{
   if (row.empty())
   {
      Expects(!"Cannot layout an empty row.");
      return;
   }

   const std::uintmax_t bytesInRow = ComputeBytesInRow(row, /*candidateSize =*/ 0);

   Block land = CalculateRowBounds(
      bytesInRow,
      row.front()->GetParent()->GetData(),
      /*updateOffset =*/ true);

   Expects(land.HasVolume());

   const auto nodeCount = row.size();

   double additionalCoverage = 0.0;

   for (auto* const node : row)
   {
      VizBlock& data = node->GetData();

      const std::uintmax_t nodeFileSize = data.file.size;
      if (nodeFileSize == 0)
      {
         Expects(!"Found a node without a file size!");
         return;
      }

      const double percentageOfParent =
         static_cast<double>(nodeFileSize) / static_cast<double>(bytesInRow);

      additionalCoverage = (land.GetWidth() > std::abs(land.GetDepth()))
         ? SlicePerpendicularToWidth(land, percentageOfParent, data, nodeCount)
         : SlicePerpendicularToDepth(land, percentageOfParent, data, nodeCount);

      Expects(additionalCoverage > 0);
      Expects(data.block.HasVolume());

      land.IncreaseCoverageBy(additionalCoverage);
   }
}

SquarifiedTreeMap::SquarifiedTreeMap(
   std::unique_ptr<FileMonitorImpl> fileMonitor,
   const std::experimental::filesystem::path& path)
   :
   VisualizationModel{ std::move(fileMonitor), path }
{
}

void SquarifiedTreeMap::Parse(const std::shared_ptr<Tree<VizBlock>>& theTree)
{
   if (!theTree)
   {
      Expects(!"Whoops, no tree in sight!");
      return;
   }

   m_fileTree = theTree;

   Stopwatch<std::chrono::milliseconds>([&]
   {
      VisualizationModel::SortNodes(*m_fileTree);
   },
   [] (const auto& elapsed, const auto& units) noexcept
   {
      spdlog::get(Constants::Logging::DEFAULT_LOG)->info(
         fmt::format("Sorted tree in: {} {}", elapsed.count(), units));
   });

   const Block rootBlock
   {
      PrecisePoint{ },
      static_cast<double>(Constants::Visualization::ROOT_BLOCK_WIDTH),
      static_cast<double>(Constants::Visualization::BLOCK_HEIGHT),
      static_cast<double>(Constants::Visualization::ROOT_BLOCK_DEPTH)
   };

   m_fileTree->GetRoot()->GetData().block = rootBlock;

   Stopwatch<std::chrono::milliseconds>([&]
   {
      SquarifyRecursively(*m_fileTree->GetRoot());
   },
   [] (const auto& elapsed, const auto& units) noexcept
   {
      spdlog::get(Constants::Logging::DEFAULT_LOG)->info(
         fmt::format("Visualization Generated in: {} {}", elapsed.count(), units));
   });

   m_hasDataBeenParsed = true;
}
