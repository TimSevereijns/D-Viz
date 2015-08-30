#include "squarifiedTreemap.h"

#include <algorithm>
#include <assert.h>
#include <iostream>
#include <limits>
#include <numeric>

namespace
{
   /**
    * @brief PrintRow prints the items in the current row.
    *
    * @param[in] row                The row to be printed.
    */
   void PrintRow(const std::vector<TreeNode<VizNode>*>& row)
   {
      std::cout << "\tRow: ";

      for (const TreeNode<VizNode>* node : row)
      {
         std::wcout << node->GetData().m_file.m_name << " ";
      }

      std::cout << std::endl;
   }

   /**
    * @brief PruneNodes removes nodes whose corresponding file or directory size is zero.
    *
    * @param[in/out] tree           The tree to be pruned.
    */
   void PruneNodes(Tree<VizNode>& tree)
   {
      std::cout << "Nodes before pruning: " << tree.Size(*tree.GetHead()) << std::endl;

      unsigned int nodesRemoved = 0;
      for (TreeNode<VizNode>& node : tree)
      {
         if (node.GetData().m_file.m_size == 0)
         {
            node.RemoveFromTree();
            nodesRemoved++;
         }
      }

      std::cout << "Nodes removed: " << nodesRemoved << std::endl;
      std::cout << "Nodes after pruning: " << tree.Size(*tree.GetHead()) << std::endl;
   }

   /**
    * @brief SortNodes traverses the tree in a post-order fashion, sorting the children of each node
    * by their respective file sizes.
    *
    * @param[in/out] tree           Th tree to be sorted.
    */
   void SortNodes(Tree<VizNode>& tree)
   {
      for (TreeNode<VizNode>& node : tree)
      {
         node.SortChildren([] (const TreeNode<VizNode>& lhs, const TreeNode<VizNode>& rhs)
            { return lhs.GetData().m_file.m_size > rhs.GetData().m_file.m_size; });
      }
   }

   /**
    * @brief RowSizeInBytes computes the total disk space represented by the nodes in the row.
    *
    * @param[in] row                The nodes in the whose size is to contribute to total row size.
    * @param[in] candidateItem      An optional additional item to be included in the row.
    *
    * @returns a total row size in bytes of disk space occupied.
    */
   std::uintmax_t RowSizeInBytes(const std::vector<TreeNode<VizNode>*>& row,
      const TreeNode<VizNode>* candidateItem)
   {
      std::uintmax_t sumOfFileSizes = std::accumulate(std::begin(row), std::end(row),
         std::uintmax_t{0}, [] (const std::uintmax_t result, const TreeNode<VizNode>* node)
      {
         return result + node->GetData().m_file.m_size;
      });

      if (candidateItem)
      {
         sumOfFileSizes += candidateItem->GetData().m_file.m_size;
      }

      return sumOfFileSizes;
   }

   /**
    * @brief CalculateRowBounds computes the outer bounds (including the necessary boundary padding)
    * needed for the blocks in the row.
    *
    * @param[in] row                The nodes to be laid out as blocks in the current row.
    * @param[in] candidate          The latest candidate to be considered for inclusion in the row.
    * @param[in] parentNode         The node on top of which the new row is to be placed.
    * @param[in] updateOffset       Whether the origin of the next row should be computed. This
    *                               should only be set to true only when the row bounds are computed
    *                               for the last time as part of row layout.
    *
    * @returns a block denoting the outer dimensions of the row boundary.
    */
   Block CalculateRowBounds(const std::vector<TreeNode<VizNode>*>& row,
      const TreeNode<VizNode>* candidate, VizNode& parentNode, const bool updateOffset)
   {
      const Block& parentBlock = parentNode.m_block;

      if (!parentBlock.IsDefined())
      {
         assert(!"Parent block is not defined!");
      }

      const QVector3D nearCorner
      {
         parentBlock.m_nextRowOrigin.x(),
         parentBlock.m_nextRowOrigin.y(),
         parentBlock.m_nextRowOrigin.z()
      };

      const QVector3D farCorner
      {
         parentBlock.GetOriginPlusHeight().x() + static_cast<float>(parentBlock.m_width),
         parentBlock.GetOriginPlusHeight().y(),
         parentBlock.GetOriginPlusHeight().z() - static_cast<float>(parentBlock.m_depth)
      };

      const Block remainingLand
      {
         nearCorner,
         farCorner.x() - nearCorner.x(),
         Visualization::BLOCK_HEIGHT,
         farCorner.z() - nearCorner.z()
      };

      const double parentBlockArea = parentBlock.m_width * parentBlock.m_depth;
      const double remainingLandArea = std::abs(remainingLand.m_width * remainingLand.m_depth);
      const double parentBytesToFill = (remainingLandArea / parentBlockArea) * parentNode.m_file.m_size;
      const std::uintmax_t rowSizeInBytes = RowSizeInBytes(row, candidate);
      const double rowToParentRatio = rowSizeInBytes / parentBytesToFill;

      Block rowRealEstate;
      if (remainingLand.m_width > std::abs(remainingLand.m_depth))
      {
         rowRealEstate = Block(QVector3D(nearCorner),
            remainingLand.m_width * rowToParentRatio,
            remainingLand.m_height,
            -remainingLand.m_depth);

         if (updateOffset)
         {
            const QVector3D nextRowOffset
            {
               static_cast<float>(rowRealEstate.m_width),
               0.0, // Height
               0.0  // Depth
            };

            parentNode.m_block.m_nextRowOrigin = nearCorner + nextRowOffset;
         }
      }
      else
      {
         rowRealEstate = Block(QVector3D(nearCorner),
            remainingLand.m_width,
            remainingLand.m_height,
            -remainingLand.m_depth * rowToParentRatio);

         if (updateOffset)
         {
            const QVector3D nextRowOffset
            {
               0.0, // Width
               0.0, // Height
               static_cast<float>(-rowRealEstate.m_depth)
            };

            parentNode.m_block.m_nextRowOrigin = nearCorner + nextRowOffset;
         }
      }

      if (!rowRealEstate.IsDefined())
      {
         assert(!"No real estate created!");
      }

      return rowRealEstate;
   }

   /**
    * @brief LayoutStrip slices the real estate into file size proportional pieces.
    *
    * @param[in] row                The items to be placed in the current piece of real estate.
    */
   void LayoutRow(std::vector<TreeNode<VizNode>*>& row)
   {
      if (row.empty())
      {
         assert(!"The row to be laid out is non-existent!");
         return;
      }

      Block& land = CalculateRowBounds(row, /*candidate =*/ nullptr,
         row.front()->GetParent()->GetData(), /*updateOffset =*/ true);

      if (!land.IsDefined())
      {
         assert(!"No land to build upon!");
         return;
      }

      const size_t nodeCount = row.size();
      const std::uintmax_t rowFileSize = RowSizeInBytes(row, /*candidate =*/ nullptr);

      double additionalCoverage = 0.0;

      double debug_consumedWidth = 0.0;
      double debug_consumedDepth = 0.0;

      for (TreeNode<VizNode>* node : row)
      {
         VizNode& data = node->GetData();
         const std::uintmax_t fileSize = data.m_file.m_size;

         if (fileSize == 0)
         {
            assert(!"Found a node without a size!");
            return;
         }

         const double percentageOfParent =
            static_cast<float>(fileSize) / static_cast<float>(rowFileSize);

         if (land.m_width > std::abs(land.m_depth))
         {
            const double blockWidthPlusPadding = land.m_width * percentageOfParent;
            const double ratioBasedPadding = ((land.m_width * 0.1) / nodeCount) / 2.0;

            double widthPaddingPerSide = std::min(ratioBasedPadding, Visualization::MAX_PADDING);
            double finalBlockWidth = blockWidthPlusPadding - (2.0 * widthPaddingPerSide);
            if (finalBlockWidth < 0)
            {
               finalBlockWidth = blockWidthPlusPadding * Visualization::PADDING_RATIO;
               widthPaddingPerSide = (blockWidthPlusPadding * (1.0 - Visualization::PADDING_RATIO)) / 2.0;
            }

            const double ratioBasedBlockDepth = std::abs(land.m_depth * Visualization::PADDING_RATIO);
            const double depthPaddingPerSide = std::min((land.m_depth - ratioBasedBlockDepth) / 2.0,
               Visualization::MAX_PADDING);

            const double finalBlockDepth = (depthPaddingPerSide == Visualization::MAX_PADDING)
               ? std::abs(land.m_depth) - (2.0 * Visualization::MAX_PADDING)
               : ratioBasedBlockDepth;

            const QVector3D offset
            {
               static_cast<float>((land.m_width * land.m_percentCovered) + widthPaddingPerSide),
               0.0, // If you want vertical spacing between block, increase this value...
               static_cast<float>(-depthPaddingPerSide)
            };

            data.m_block = Block(land.m_vertices.front() + offset,
               finalBlockWidth,
               Visualization::BLOCK_HEIGHT,
               finalBlockDepth
            );

            additionalCoverage = blockWidthPlusPadding / land.m_width;

            debug_consumedWidth += blockWidthPlusPadding;
            if (debug_consumedWidth > land.m_width * 1.001) // For rounding errors greater than 0.1%
            {
               assert(!"Found a ridiculous rounding error!");
            }
         }
         else
         {
            const double blockDepthPlusPadding = std::abs(land.m_depth * percentageOfParent);
            const double ratioBasedPadding = (land.m_depth * 0.1) / nodeCount / 2.0;

            double depthPaddingPerSide = std::min(ratioBasedPadding, Visualization::MAX_PADDING);
            double finalBlockDepth = blockDepthPlusPadding - (2.0 * depthPaddingPerSide);
            if (finalBlockDepth < 0)
            {
               finalBlockDepth = blockDepthPlusPadding * Visualization::PADDING_RATIO;
               depthPaddingPerSide = (blockDepthPlusPadding * (1.0 - Visualization::PADDING_RATIO)) / 2.0;
            }

            const double ratioBasedWidth = land.m_width * Visualization::PADDING_RATIO;
            const double widthPaddingPerSide = std::min((land.m_width - ratioBasedWidth) / 2.0,
               Visualization::MAX_PADDING);

            const double finalBlockWidth = (widthPaddingPerSide == Visualization::MAX_PADDING)
               ? land.m_width - (2.0 * Visualization::MAX_PADDING)
               : ratioBasedWidth;

            const QVector3D offset
            {
               static_cast<float>(widthPaddingPerSide),
               0.0, // If you want vertical spacing between block, increase this value...
               static_cast<float>(-(land.m_depth * land.m_percentCovered) - depthPaddingPerSide)
            };

            data.m_block = Block(land.m_vertices.front() + offset,
               finalBlockWidth,
               Visualization::BLOCK_HEIGHT,
               std::abs(finalBlockDepth)
            );

            additionalCoverage = blockDepthPlusPadding / land.m_depth;

            debug_consumedDepth += blockDepthPlusPadding;
            if (debug_consumedDepth > land.m_depth * 1.001) // For rounding errors greater than 0.1%
            {
               assert(!"Found a ridiculous rounding error!");
            }
         }

         if (!data.m_block.IsDefined())
         {
            assert(!"Block is not defined!");
         }

         if (additionalCoverage == 0.0)
         {
            assert(!"Node provided no additional coverage!");
         }

         land.m_percentCovered += additionalCoverage;
      }
   }

   /**
    * @brief ComputeWorstAspectRatio calculates the worst aspect ratio of all items accepted into
    * the row along with one optional candidate item.
    *
    * @param[in] row                   The nodes that have been placed in the current real estate.
    * @param[in] candidateItem         One additional item that is to be considered for inclusion
    *                                  into the same row. Can be nullptr.
    * @param[in] lengthOfShortestSide  Length of shortest side of the enclosing row's boundary.
    *
    * @returns a float representing the aspect ration that farthest from optimal (i.e.: square).
    */
   double ComputeWorstAspectRatio(const std::vector<TreeNode<VizNode>*>& row,
      const TreeNode<VizNode>* candidateItem, const float shortestSideOfRow, VizNode& parentNode)
   {
      if (row.empty() && !candidateItem)
      {
         return std::numeric_limits<double>::max();
      }

      const Block rowBounds = CalculateRowBounds(row, candidateItem, parentNode,
         /*updateOffset =*/ false);
      const double totalRowSurfaceArea = std::abs(rowBounds.m_width * rowBounds.m_depth);
      const std::uintmax_t totalRowSizeInBytes = RowSizeInBytes(row, candidateItem);

      // Find the largest surface area if the row and candidate were laid out:

      std::uintmax_t largestNodeSizeInBytes;
      if (candidateItem && !row.empty())
      {
         largestNodeSizeInBytes = std::max(row.front()->GetData().m_file.m_size,
            candidateItem->GetData().m_file.m_size);
      }
      else if (candidateItem)
      {
         largestNodeSizeInBytes = candidateItem->GetData().m_file.m_size;
      }
      else
      {
         largestNodeSizeInBytes = row.front()->GetData().m_file.m_size;
      }
      const double largestSurface =
         (static_cast<double>(largestNodeSizeInBytes) / static_cast<double>(totalRowSizeInBytes)) *
         totalRowSurfaceArea;

      // Find the smallest surface area if the row and candidate were laid out:

      std::uintmax_t smallestNodeSizeInBytes;
      if (candidateItem && !row.empty())
      {
         smallestNodeSizeInBytes = std::min(row.back()->GetData().m_file.m_size,
            candidateItem->GetData().m_file.m_size);
      }
      else if (candidateItem)
      {
         smallestNodeSizeInBytes = candidateItem->GetData().m_file.m_size;
      }
      else
      {
         smallestNodeSizeInBytes = row.back()->GetData().m_file.m_size;
      }
      const double smallestElement =
         (static_cast<double>(smallestNodeSizeInBytes) / static_cast<double>(totalRowSizeInBytes)) *
         totalRowSurfaceArea;

      // Now compute the worst aspect ratio between the two choices above:

      const double lengthSquared = shortestSideOfRow * shortestSideOfRow;
      const double areaSquared = totalRowSurfaceArea * totalRowSurfaceArea;

      const double worstRatio = std::max((lengthSquared * largestSurface) / (areaSquared),
         (areaSquared) / (lengthSquared * smallestElement));

      return worstRatio;
   }

   /**
    * @brief Squarify is the main function that drives the parsing and creation of a simplified
    * squarified tree map.
    *
    * @param[in] nodes              The first node in the tree to be laid out. This node must have
    *                               a parent; this parent can be a dummy node to kick things off.
    */
   void Squarify(TreeNode<VizNode>& node)
   {
      TreeNode<VizNode>* currentNode = &node;
      TreeNode<VizNode>* firstChild = currentNode;
      std::vector<TreeNode<VizNode>*> currentRow;

      while (currentNode)
      {
         TreeNode<VizNode>* parentNode = &*currentNode->GetParent();
         assert(parentNode);

         const double shortestSide = std::min(parentNode->GetData().m_block.m_width,
            parentNode->GetData().m_block.m_depth);

         // If worst aspect ratio improves, add block to current row:
         if (ComputeWorstAspectRatio(currentRow, currentNode, shortestSide, parentNode->GetData()) <=
             ComputeWorstAspectRatio(currentRow, nullptr,     shortestSide, parentNode->GetData()))
         {
            currentRow.emplace_back(currentNode);
         }
         else // if aspect ratio gets worse, layout current row and create a new row:
         {
            LayoutRow(currentRow);

            currentRow.clear();
            currentRow.emplace_back(currentNode);
         }

         currentNode = &*currentNode->GetNextSibling();
      }

      if (!currentRow.empty())
      {
         LayoutRow(currentRow);
      }

      currentNode = firstChild;

      while (currentNode)
      {
         Squarify(*currentNode->GetFirstChild());
         currentNode = &*currentNode->GetNextSibling();
      }
   }

   /**
    * @brief SquarifyProper
    * @param node
    * @param currentRow
    * @param shortestSide
    */
   void SquarifyProper(TreeNode<VizNode>* node, std::vector<TreeNode<VizNode>*> currentRow,
      const double shortestSide)
   {
      if (!node)
      {
         return;
      }

      // If worst aspect ratio improves, add block to current row:
      if (ComputeWorstAspectRatio(currentRow, node, shortestSide, node->GetParent()->GetData()) <=
          ComputeWorstAspectRatio(currentRow, nullptr, shortestSide, node->GetParent()->GetData()))
      {
         currentRow.emplace_back(node);
         SquarifyProper(&*node->GetNextSibling(), currentRow, shortestSide);
      }
      else
      {
         if (!currentRow.empty())
         {
            LayoutRow(currentRow);
         }

         TreeNode<VizNode>* parentNode = &*node->GetParent();
         if (parentNode)
         {
            return;
         }

         const double newShortestSide = std::min(parentNode->GetData().m_block.m_width,
            parentNode->GetData().m_block.m_depth);

         auto emptyRow = std::vector<TreeNode<VizNode>*>();

         SquarifyProper(node, emptyRow, newShortestSide);
      }
   }
}

SquarifiedTreeMap::SquarifiedTreeMap(const VisualizationParameters& parameters)
   : Visualization(parameters)
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

   PruneNodes(*theTree);
   SortNodes(*theTree);

   const Block rootBlock
   {
      QVector3D(0, 0, 0),
      Visualization::ROOT_BLOCK_WIDTH,
      Visualization::BLOCK_HEIGHT,
      Visualization::ROOT_BLOCK_DEPTH
   };

   theTree->GetHead()->GetData().m_block = rootBlock;

   const auto startParseTime = std::chrono::high_resolution_clock::now();
   Squarify(*theTree->GetHead()->GetFirstChild());
   const auto endParseTime = std::chrono::high_resolution_clock::now();

   auto parsingTime =
      std::chrono::duration_cast<std::chrono::duration<double>>(endParseTime - startParseTime);

   std::cout << "Parse time (in seconds): " << parsingTime.count() << std::endl;

   m_hasDataBeenParsed = true;
}
