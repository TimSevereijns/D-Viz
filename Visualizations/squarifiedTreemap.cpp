#include "squarifiedTreemap.h"

#include <algorithm>
#include <assert.h>
#include <limits>
#include <numeric>

#include "../ThirdParty/stopwatch.hpp"

namespace
{
   /**
    * @brief RowSizeInBytes computes the total disk space represented by the nodes in the row.
    *
    * @param[in] row                The nodes in the whose size is to contribute to total row size.
    * @param[in] candidateItem      An optional additional item to be included in the row.
    *
    * @returns A total row size in bytes of disk space occupied.
    */
   std::uintmax_t ComputeBytesInRow(const std::vector<TreeNode<VizNode>*>& row,
      const std::uintmax_t candidateSize)
   {
      std::uintmax_t sumOfFileSizes = std::accumulate(std::begin(row), std::end(row),
         std::uintmax_t{ 0 }, [] (const std::uintmax_t result, const TreeNode<VizNode>* node)
      {
         return result + (*node)->file.size;
      });

      sumOfFileSizes += candidateSize;

      return sumOfFileSizes;
   }

   /**
    * @brief ComputeRemainingArea computes the area of the specified block that remains available
    * to be built upon.
    *
    * @param[in] block              The block to build upon.
    */
   Block ComputeRemainingArea(const Block& block)
   {
      const DoublePoint3D nearCorner
      {
         block.nextRowOrigin.x(),
         block.nextRowOrigin.y(),
         block.nextRowOrigin.z()
      };

      const DoublePoint3D farCorner
      {
         block.GetNextChildOrigin().x() + block.width,
         block.GetNextChildOrigin().y(),
         block.GetNextChildOrigin().z() - block.depth
      };

      const Block remainingArea
      {
         nearCorner,                         // Origin
         farCorner.x() - nearCorner.x(),     // Width
         Visualization::BLOCK_HEIGHT,        // Height
         farCorner.z() - nearCorner.z()      // Depth
      };

      if (!remainingArea.HasVolume())
      {
         assert(!"Whoops. No remaining area left.");
      }

      return remainingArea;
   }

   /**
    * @brief CalculateRowBounds computes the outer bounds (including the necessary boundary padding)
    * needed to properly contain the row once laid out on top of its parent node.
    *
    * @param[in] row                The nodes to be laid out as blocks in the current row.
    * @param[in] candidateSize      The size of the latest candidate to be considered for inclusion
    *                               in the row.
    * @param[in, out] parentNode    The node on top of which the new row is to be placed.
    * @param[in] updateOffset       Whether the origin of the next row should be computed. This
    *                               should only be set to true only when the row bounds are computed
    *                               for the last time as part of row layout.
    *
    * @returns A block representing the outer dimensions of the row boundary.
    */
   Block CalculateRowBounds(
      const std::vector<TreeNode<VizNode>*>& row,
      const std::uintmax_t candidateSize,
      VizNode& parentNode,
      const bool updateOffset)
   {
      const Block& parentBlock = parentNode.block;

      if (!parentBlock.HasVolume())
      {
         assert(!"Parent block is not defined!");
      }

      Block remainingLand = ComputeRemainingArea(parentBlock);

      const double parentArea = parentBlock.width * parentBlock.depth;
      const double remainingArea = std::abs(remainingLand.width * remainingLand.depth);
      const double remainingBytes = (remainingArea / parentArea) * parentNode.file.size;

      const std::uintmax_t rowSizeInBytes = ComputeBytesInRow(row, candidateSize);
      const double rowToParentRatio = rowSizeInBytes / remainingBytes;

      const DoublePoint3D nearCorner
      {
         parentBlock.nextRowOrigin.x(),
         parentBlock.nextRowOrigin.y(),
         parentBlock.nextRowOrigin.z()
      };

      Block rowRealEstate;
      if (remainingLand.width > std::abs(remainingLand.depth))
      {
         rowRealEstate = Block
         {
            nearCorner,
            remainingLand.width * rowToParentRatio,
            remainingLand.height,
            -remainingLand.depth
         };

         if (updateOffset)
         {
            const DoublePoint3D nextRowOffset
            {
               rowRealEstate.width,
               0.0, // Height
               0.0  // Depth
            };

            parentNode.block.nextRowOrigin = nearCorner + nextRowOffset;
         }
      }
      else
      {
         rowRealEstate = Block
         {
            DoublePoint3D(nearCorner),
            remainingLand.width,
            remainingLand.height,
            -remainingLand.depth * rowToParentRatio
         };

         if (updateOffset)
         {
            const DoublePoint3D nextRowOffset
            {
               0.0, // Width
               0.0, // Height
               -rowRealEstate.depth
            };

            parentNode.block.nextRowOrigin = nearCorner + nextRowOffset;
         }
      }

      assert(rowRealEstate.HasVolume());

      return rowRealEstate;
   }

   /**
    * @brief SlicePerpendicularToWidth
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
      VizNode& node,
      const size_t nodeCount)
   {
      const double blockWidthPlusPadding = land.width * percentageOfParent;
      const double ratioBasedPadding = ((land.width * 0.1) / nodeCount) / 2.0;

      double widthPaddingPerSide = std::min(ratioBasedPadding, Visualization::MAX_PADDING);
      double finalBlockWidth = blockWidthPlusPadding - (2.0 * widthPaddingPerSide);
      if (finalBlockWidth < 0)
      {
         finalBlockWidth = blockWidthPlusPadding * Visualization::PADDING_RATIO;
         widthPaddingPerSide = (blockWidthPlusPadding * (1.0 - Visualization::PADDING_RATIO)) / 2.0;
      }

      const double ratioBasedBlockDepth = std::abs(land.depth * Visualization::PADDING_RATIO);
      const double depthPaddingPerSide = std::min((land.depth - ratioBasedBlockDepth) / 2.0,
         Visualization::MAX_PADDING);

      const double finalBlockDepth = (depthPaddingPerSide == Visualization::MAX_PADDING)
         ? std::abs(land.depth) - (2.0 * Visualization::MAX_PADDING)
         : ratioBasedBlockDepth;

      const DoublePoint3D offset
      {
         (land.width * land.percentCovered) + widthPaddingPerSide,
         0.0,
         -depthPaddingPerSide
      };

      node.block = Block
      {
         land.origin + offset,
         finalBlockWidth,
         Visualization::BLOCK_HEIGHT,
         finalBlockDepth
      };

      const double additionalCoverage = blockWidthPlusPadding / land.width;
      return additionalCoverage;
   }

   /**
    * @brief SlicePerpendicularToDepth
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
      VizNode& node,
      const size_t nodeCount)
   {
      const double blockDepthPlusPadding = std::abs(land.depth * percentageOfParent);
      const double ratioBasedPadding = (land.depth * 0.1) / nodeCount / 2.0;

      double depthPaddingPerSide = std::min(ratioBasedPadding, Visualization::MAX_PADDING);
      double finalBlockDepth = blockDepthPlusPadding - (2.0 * depthPaddingPerSide);
      if (finalBlockDepth < 0)
      {
         finalBlockDepth = blockDepthPlusPadding * Visualization::PADDING_RATIO;
         depthPaddingPerSide = (blockDepthPlusPadding * (1.0 - Visualization::PADDING_RATIO)) / 2.0;
      }

      const double ratioBasedWidth = land.width * Visualization::PADDING_RATIO;
      const double widthPaddingPerSide = std::min((land.width - ratioBasedWidth) / 2.0,
         Visualization::MAX_PADDING);

      const double finalBlockWidth = (widthPaddingPerSide == Visualization::MAX_PADDING)
         ? land.width - (2.0 * Visualization::MAX_PADDING)
         : ratioBasedWidth;

      const DoublePoint3D offset
      {
         widthPaddingPerSide,
         0.0,
         -(land.depth * land.percentCovered) - depthPaddingPerSide
      };

      node.block = Block
      {
         land.origin + offset,
         finalBlockWidth,
         Visualization::BLOCK_HEIGHT,
         std::abs(finalBlockDepth)
      };

      const double additionalCoverage = blockDepthPlusPadding / land.depth;
      return additionalCoverage;
   }

   /**
    * @brief LayoutRow takes all the nodes that are to be included in a single row and then
    * constructs the individual blocks representing the nodes in that row so that the bounds of the
    * row are subdivided along the longest axis of available space.
    *
    * @param[in, out] row           The nodes to include in a single row.
    */
   void LayoutRow(std::vector<TreeNode<VizNode>*>& row)
   {
      if (row.empty())
      {
         assert(!"Cannot layout an empty row.");
         return;
      }

      Block& land = CalculateRowBounds(row, /*candidateSize =*/ 0,
         row.front()->GetParent()->GetData(), /*updateOffset =*/ true);

      assert(land.HasVolume());

      const size_t nodeCount = row.size();
      const std::uintmax_t rowFileSize = ComputeBytesInRow(row, /*candidateSize =*/ 0);

      double additionalCoverage = 0.0;

      for (TreeNode<VizNode>* const node : row)
      {
         VizNode& data = node->GetData();

         const std::uintmax_t nodeFileSize = data.file.size;
         if (nodeFileSize == 0)
         {
            assert(!"Found a node without a file size!");
            return;
         }

         const double percentageOfParent =
            static_cast<double>(nodeFileSize) / static_cast<double>(rowFileSize);

         additionalCoverage = (land.width > std::abs(land.depth))
            ? SlicePerpendicularToWidth(land, percentageOfParent, data, nodeCount)
            : SlicePerpendicularToDepth(land, percentageOfParent, data, nodeCount);

         assert(additionalCoverage > 0);
         assert(data.block.HasVolume());

         land.percentCovered += additionalCoverage;
      }
   }

   /**
    * @brief ComputeShortestEdgeOfRemainingArea calculates the shortest dimension (width or depth)
    * of the remaining bounds available to build within.
    *
    * @param[in] node               The node being built upon.
    *
    * @returns A double respresent the length of the shortest edge.
    */
   double ComputeShortestEdgeOfRemainingBounds(const VizNode& node)
   {
      const Block remainingRealEstate = ComputeRemainingArea(node.block);
      const auto shortestEdge = std::min(std::abs(remainingRealEstate.depth),
         std::abs(remainingRealEstate.width));

      assert(shortestEdge > 0);
      return shortestEdge;
   }

   /**
    * @brief ComputeWorstAspectRatio calculates the worst aspect ratio of all items accepted into
    * the row along with one optional candidate item.
    *
    * @param[in] row                   The nodes that have been placed in the current real estate.
    * @param[in] candidateSize         The size of the candidate node that is to be considered for
    *                                  inclusion in the current row. Zero is no candidate necessary.
    * @param[in] shortestEdgeOfBounds  Length of shortest side of the enclosing row's boundary.
    *
    * @returns A double representing the least square aspect ratio.
    */
   double ComputeWorstAspectRatio(
      const std::vector<TreeNode<VizNode>*>& row,
      const std::uintmax_t candidateSize,
      VizNode& parentNode,
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

      const bool updateOffset = false;
      const Block rowBounds = CalculateRowBounds(row, candidateSize, parentNode, updateOffset);

      const double totalRowArea = std::abs(rowBounds.width * rowBounds.depth);

      const std::uintmax_t totalRowSize = ComputeBytesInRow(row, candidateSize);

      const double largestArea =
         (static_cast<double>(largestNodeInBytes) / static_cast<double>(totalRowSize)) *
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
      assert(totalRowSize > 0);
      assert(totalRowArea > 0);

      const double smallestArea =
         (static_cast<double>(smallestNodeInBytes) / static_cast<double>(totalRowSize)) *
         totalRowArea;

      // Now compute the worst aspect ratio between the two choices above:

      const double lengthSquared = shortestEdgeOfBounds * shortestEdgeOfBounds;
      const double areaSquared = totalRowArea * totalRowArea;

      const double worstRatio = std::max((lengthSquared * largestArea) / (areaSquared),
         (areaSquared) / (lengthSquared * smallestArea));

      return worstRatio;
   }

   /**
    * @brief SquarifyAndLayoutRows represents the heart of the algorithm and decides which nodes
    * ought to be added to which row in order to acheive an acceptable layout.
    *
    * @param[in, out] nodes         The sibling nodes to be laid out within the available bounds
    *                               of the parent node.
    */
   void SquarifyAndLayoutRows(const std::vector<TreeNode<VizNode>*>& nodes)
   {
      if (nodes.empty())
      {
         return;
      }

      TreeNode<VizNode>* parentNode = nodes.front()->GetParent();
      assert(parentNode);

      VizNode& parentVizNode = parentNode->GetData();
      assert(parentVizNode.block.HasVolume() && parentVizNode.block.IsNotInverted());

      std::vector<TreeNode<VizNode>*> row;

      double shortestEdgeOfBounds = ComputeShortestEdgeOfRemainingBounds(parentVizNode);
      assert(shortestEdgeOfBounds > 0.0);

      for (TreeNode<VizNode>* const node : nodes)
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

   /**
    * @brief SquarifyRecursively is the main entry point into the squarification algorithm, and
    * performs a recursive breadth-first traversal of the node tree and lays out the children of
    * each node with the aid of various helper functions.
    *
    * @param[in, out] root          The node whose children to lay out.
    */
   void SquarifyRecursively(const TreeNode<VizNode>& root)
   {
      TreeNode<VizNode>* firstChild = root.GetFirstChild();
      if (!firstChild)
      {
         return;
      }

      std::vector<TreeNode<VizNode>*> children;
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
}

SquarifiedTreeMap::SquarifiedTreeMap(const VisualizationParameters& parameters) :
   Visualization{ parameters }
{
}

void SquarifiedTreeMap::Parse(const std::shared_ptr<Tree<VizNode>>& theTree)
{
   if (!theTree)
   {
      assert(!"Whoops, no tree in sight!");
      return;
   }

   m_theTree = theTree;

   TIME_IN_MILLISECONDS(
      Visualization::SortNodes(*m_theTree),
      "Sorted tree in ");

   TIME_IN_MILLISECONDS(
      FindSmallestandLargestDirectory(*m_theTree),
      "Found smallest and largest directories in ");

   const Block rootBlock
   {
      DoublePoint3D{ },
      Visualization::ROOT_BLOCK_WIDTH,
      Visualization::BLOCK_HEIGHT,
      Visualization::ROOT_BLOCK_DEPTH
   };

   theTree->GetHead()->GetData().block = rootBlock;

   TIME_IN_MILLISECONDS(
      SquarifyRecursively(*theTree->GetHead()),
      "Visualization generated in ");

   m_hasDataBeenParsed = true;
}
