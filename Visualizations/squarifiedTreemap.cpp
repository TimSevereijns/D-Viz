#include "squarifiedTreemap.h"

#include <algorithm>
#include <assert.h>
#include <iostream>
#include <limits>
#include <numeric>

namespace
{
   /**
    * @brief PrintRow
    * @param row
    */
   void PrintRow(std::vector<TreeNode<VizNode>*>& row)
   {
      std::cout << "\tRow: ";

      for (const TreeNode<VizNode>* node : row)
      {
         std::wcout << node->GetData().m_file.m_name << " ";
      }

      std::cout << std::endl;
   }

   /**
    * @brief RowSizeInBytes computes the total disk space represented by the nodes in the row.
    *
    * @param[in] row                The nodes in the whose size is to contribute to total row size.
    * TODO
    *
    * @returns a total row size in bytes of disk space occupied.
    */
   std::uintmax_t RowSizeInBytes(std::vector<TreeNode<VizNode>*>& row,
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
    * @param[in] candidate
    * @param[in] parentNode
    * @param[in] updateOffset
    *
    * @returns a block denoting the outer dimensions of the row boundary.
    */
   Block CalculateRowBounds(std::vector<TreeNode<VizNode>*>& row,
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
         parentBlock.GetOriginPlusHeight().x() + parentBlock.m_width,
         parentBlock.GetOriginPlusHeight().y(),
         parentBlock.GetOriginPlusHeight().z() - parentBlock.m_depth
      };

      const Block remainingLand
      {
         nearCorner,
         farCorner.x() - nearCorner.x(),
         Visualization::BLOCK_HEIGHT,
         farCorner.z() - nearCorner.z()
      };

      const float parentBlockArea = parentBlock.m_width * parentBlock.m_depth;
      const float remainingLandArea = std::abs(remainingLand.m_width * remainingLand.m_depth);
      const float parentBytesToFill = (remainingLandArea / parentBlockArea) * parentNode.m_file.m_size;
      const float rowToParentRatio = RowSizeInBytes(row, candidate) / parentBytesToFill;

      Block rowRealEstate;
      if (remainingLand.m_width > std::abs(remainingLand.m_depth))
      {
         rowRealEstate = Block(QVector3D(nearCorner),
            remainingLand.m_width * rowToParentRatio,
            remainingLand.m_height,
            -remainingLand.m_depth);

         if (updateOffset)
         {
            const QVector3D nextChildOffset
            {
               rowRealEstate.m_width,
               0.0f, // Height
               0.0f  // Depth
            };

            parentNode.m_block.m_nextRowOrigin = nearCorner + nextChildOffset;
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
            const QVector3D nextChildOffset
            {
               0.0f, // Width
               0.0f, // Height
               -rowRealEstate.m_depth
            };

            parentNode.m_block.m_nextRowOrigin = nearCorner + nextChildOffset;
         }
      }

      // TODO: Add assert to catch the rare block that exceeds its bounds.

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
         assert(!"The row to be laid out is nonexistent!");
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

      float additionalCoverage = 0.0f;

      for (TreeNode<VizNode>* node : row)
      {
         VizNode& data = node->GetData();
         const std::uintmax_t fileSize = data.m_file.m_size;

         if (fileSize == 0)
         {
            assert(!"Found a node without a size!");
            return;
         }

         const float percentageOfParent = static_cast<float>(fileSize) /
            static_cast<float>(rowFileSize);

         if (land.m_width > std::abs(land.m_depth))
         {
            const auto paddedBlockWidth = land.m_width * percentageOfParent;

            auto actualBlockWidth = paddedBlockWidth - (2.0f * Visualization::DEFAULT_PADDING);
            if (actualBlockWidth <= 0.0f)
            {
               actualBlockWidth = paddedBlockWidth * Visualization::BLOCK_TO_REAL_ESTATE_RATIO;
            }

            auto widthPaddingPerSide = Visualization::DEFAULT_PADDING;
            if (widthPaddingPerSide * 2.0f >= actualBlockWidth)
            {
               widthPaddingPerSide = ((land.m_width * 0.1f) / nodeCount) / 2.0f;
            }

            auto actualBlockDepth = land.m_depth - (2.0f * Visualization::DEFAULT_PADDING);
            if (actualBlockDepth <= 0.0f)
            {
               actualBlockDepth = std::abs(land.m_depth * Visualization::BLOCK_TO_REAL_ESTATE_RATIO);
            }

            auto depthPaddingPerSide = Visualization::DEFAULT_PADDING;
            if (depthPaddingPerSide * 2.0f >= actualBlockDepth)
            {
               depthPaddingPerSide = (land.m_depth - actualBlockDepth) / 2.0f;
            }

            const QVector3D offset
            {
               (land.m_width * land.m_percentCovered) + widthPaddingPerSide,
               0.0f, // If you want vertical spacing between block, increase this value...
               -depthPaddingPerSide
            };

            data.m_block = Block(land.m_vertices.front() + offset,
               actualBlockWidth,
               Visualization::BLOCK_HEIGHT,
               actualBlockDepth
            );

            additionalCoverage = (actualBlockWidth + (2 * widthPaddingPerSide)) / land.m_width;
         }
         else
         {
            const auto paddedBlockDepth = std::abs(land.m_depth * percentageOfParent);

            auto actualBlockDepth = paddedBlockDepth - (2.0f * Visualization::DEFAULT_PADDING);
            if (actualBlockDepth <= 0.0f)
            {
               actualBlockDepth = paddedBlockDepth * Visualization::BLOCK_TO_REAL_ESTATE_RATIO;
            }

            auto depthPaddingPerSide = Visualization::DEFAULT_PADDING;
            if (depthPaddingPerSide * 2.0f >= actualBlockDepth)
            {
               depthPaddingPerSide = ((land.m_depth * 0.1f) / nodeCount) / 2.0f;
            }

            auto actualBlockWidth = land.m_width - (2.0f * Visualization::DEFAULT_PADDING);
            if (actualBlockWidth <= 0.0f)
            {
               actualBlockWidth = land.m_width * Visualization::BLOCK_TO_REAL_ESTATE_RATIO;
            }

            auto widthPaddingPerSide = Visualization::DEFAULT_PADDING;
            if (widthPaddingPerSide * 2.0f >= actualBlockWidth)
            {
               widthPaddingPerSide = (land.m_width - actualBlockWidth) / 2.0f;
            }

            const QVector3D offset
            {
               widthPaddingPerSide,
               0.0f, // If you want vertical spacing between block, increase this value...
               -(land.m_depth * land.m_percentCovered) - depthPaddingPerSide
            };

            data.m_block = Block(land.m_vertices.front() + offset,
               actualBlockWidth,
               Visualization::BLOCK_HEIGHT,
               actualBlockDepth
            );

            additionalCoverage = (actualBlockDepth + (2 * depthPaddingPerSide)) / land.m_depth;
            if (additionalCoverage == 0.0f)
            {
               assert(!"Node provided no additional coverage!");
            }
         }

         if (!data.m_block.IsDefined())
         {
            assert(!"Block is not defined!");
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
   float ComputeWorstAspectRatio(std::vector<TreeNode<VizNode>*>& row,
      const TreeNode<VizNode>* candidateItem, const float shortestSideOfRow, VizNode& parentNode)
   {
      if (row.empty() && !candidateItem)
      {
         return std::numeric_limits<float>::max();
      }

      const Block rowBounds = CalculateRowBounds(row, candidateItem, parentNode,
         /*updateOffset =*/ false);
      const float totalRowSurfaceArea = std::abs(rowBounds.m_width * rowBounds.m_depth);
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

      const float lengthSquared = shortestSideOfRow * shortestSideOfRow;
      const float areaSquared = totalRowSurfaceArea * totalRowSurfaceArea;

      const float worstRatio = std::max((lengthSquared * largestSurface) / (areaSquared),
         (areaSquared) / (lengthSquared * smallestElement));

      //std::cout << "Ratio: " << worstRatio << std::endl;

      return worstRatio;
   }

   /**
    * @brief Squarify is the main function that drives the parsing and creation of the squarified
    * tree map, as described in the paper "Squarified Treemaps," by Mark Bruls, Kees Huizing, and
    * Jarke J. van Wijk. Bedankt jongens!
    *
    * @param[in] nodes              The first node in the tree to be laid out. This node must have
    *                               a parent; this parent can be a dummy node to kick things off.
    */
   void Squarify(TreeNode<VizNode>& nodes)
   {
      TreeNode<VizNode>* currentNode = &nodes;
      TreeNode<VizNode>* firstChild = currentNode;
      std::vector<TreeNode<VizNode>*> currentRow;

      while (currentNode)
      {
         TreeNode<VizNode>* parentNode = &*currentNode->GetParent();
         assert(parentNode);

         const float shortestSide = std::min(parentNode->GetData().m_block.m_width,
            parentNode->GetData().m_block.m_depth);

         // If worst aspect ratio improves, add block to current row:
         if (ComputeWorstAspectRatio(currentRow, currentNode, shortestSide, parentNode->GetData()) <=
             ComputeWorstAspectRatio(currentRow, nullptr,     shortestSide, parentNode->GetData()))
         {
            currentRow.emplace_back(currentNode);
         }
         else // if aspect ratio gets worse, layout current row and create a new row:
         {
            //PrintRow(currentRow);
            LayoutRow(currentRow);

            currentRow.clear();
            currentRow.emplace_back(currentNode);
         }

         currentNode = &*currentNode->GetNextSibling();
      }

      if (!currentRow.empty())
      {
         //PrintRow(currentRow);
         LayoutRow(currentRow);
      }

      currentNode = firstChild;

      while (currentNode)
      {
         Squarify(*currentNode->GetFirstChild());
         currentNode = &*currentNode->GetNextSibling();
      }
   }
}

SquarifiedTreeMap::SquarifiedTreeMap(const std::wstring& rawPath)
   : Visualization(rawPath)
{
}

SquarifiedTreeMap::~SquarifiedTreeMap()
{
}

void SquarifiedTreeMap::ParseScan()
{
   Tree<VizNode>& tree = m_diskScanner.GetDirectoryTree();

   // Remove empty directories:
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

   for (TreeNode<VizNode>& node : tree)
   {
      node.SortChildren([] (const TreeNode<VizNode>& lhs, const TreeNode<VizNode>& rhs)
         { return lhs.GetData().m_file.m_size >= rhs.GetData().m_file.m_size; });
   }

   // Set the size of the root visualization:
   tree.GetHead()->GetData().m_block = Block(QVector3D(0, 0, 0),
      1000.0f, Visualization::BLOCK_HEIGHT, 1000.0f);

   const auto startParseTime = std::chrono::high_resolution_clock::now();
   Squarify(*tree.GetHead()->GetFirstChild());
   const auto endParseTime = std::chrono::high_resolution_clock::now();

   auto parsingTime =
      std::chrono::duration_cast<std::chrono::duration<double>>(endParseTime - startParseTime);

   std::cout << "Parse time (in seconds): " << parsingTime.count() << std::endl;

   m_hasDataBeenParsed = true;
}
