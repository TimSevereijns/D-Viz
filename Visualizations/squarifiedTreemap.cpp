#include "squarifiedTreemap.h"

#include <algorithm>
#include <assert.h>
#include <iostream>
#include <numeric>

namespace
{
   /**
    * @brief LayoutStrip slices the real estate into file size proportional pieces.
    * @param[in] row                The items to be placed in the current piece of real estate.
    * @param[in] realEsate          The space into which the nodes have to be placed.
    */
   void LayoutStrip(std::vector<TreeNode<VizNode>>& row, Block& realEstate)
   {
      static const float BLOCK_HEIGHT = 0.0625f;
      static const float BLOCK_TO_REAL_ESTATE_RATIO = 0.9f;

      if (row.size() == 0 || !realEstate.IsDefined())
      {
         return;
      }

      const size_t nodeCount = row.size();
      const float percentCovered = 0.0f;
      const float parentFileSize = row[0].GetParent()->GetData().m_file.m_size;

      float additionalCoverage = 0.0f;

      std::for_each(std::begin(row), std::end(row),
         [&] (TreeNode<VizNode>& node)
      {
         VizNode& data = node.GetData();
         const std::uintmax_t fileSize = data.m_file.m_size;

         if (fileSize == 0)
         {
            assert(!"Found a node without a size!");
            return;
         }

         const float percentageOfParent = static_cast<float>(fileSize) /
               static_cast<float>(parentFileSize);

         if (realEstate.m_width > realEstate.m_depth)
         {
            const auto paddedBlockWidth = realEstate.m_width * percentageOfParent;
            const auto actualBlockWidth = paddedBlockWidth * BLOCK_TO_REAL_ESTATE_RATIO;
            const auto widthPaddingPerSide = ((realEstate.m_width * 0.1f) / nodeCount) / 2.0f;

            const auto actualBlockDepth = realEstate.m_depth * BLOCK_TO_REAL_ESTATE_RATIO;
            const auto depthPaddingPerSide = (realEstate.m_height - actualBlockDepth) / 2.0f;

            const auto offset = QVector3D(
               (realEstate.m_width * percentCovered) + widthPaddingPerSide,
               BLOCK_HEIGHT,
               -depthPaddingPerSide
            );

            data.m_block = Block(realEstate.m_vertices[0] + offset,
               actualBlockWidth,
               BLOCK_HEIGHT,
               actualBlockDepth
            );

            additionalCoverage = (actualBlockWidth + (2 * widthPaddingPerSide)) /
                  realEstate.m_width;
         }
         else
         {
            const auto paddedBlockDepth = realEstate.m_depth * percentageOfParent;
            const auto actualBlockDepth = paddedBlockDepth * BLOCK_TO_REAL_ESTATE_RATIO;
            const auto depthPaddingPerSide = ((realEstate.m_depth * 0.1f) / nodeCount) / 2.0f;

            const auto actualBlockWidth = realEstate.m_width * BLOCK_TO_REAL_ESTATE_RATIO;
            const auto widthPaddingPerSide = (realEstate.m_height - actualBlockWidth) / 2.0f;

            const auto offset = QVector3D(
               widthPaddingPerSide,
               BLOCK_HEIGHT,
               -(realEstate.m_depth * percentCovered) - depthPaddingPerSide
            );

            data.m_block = Block(realEstate.m_vertices[0] + offset,
               actualBlockWidth,
               BLOCK_HEIGHT,
               actualBlockDepth
            );

            additionalCoverage = (actualBlockDepth + (2 * depthPaddingPerSide)) /
                  realEstate.m_depth;
         }

         if (additionalCoverage)
         {
            assert(!"The new block does not appear to have required dimensions!");
         }

         realEstate.m_percentCovered += additionalCoverage;
      });
   }

   /**
    * @brief ComputeWorstAspectRatio
    * @param[in] row                   The nodes that have been placed in the current real estate.
    * @param[in] additionalItem        One additional item item to be considered in the same estate.
    * @param[in] lengthOfShortestSide  Length of shortest side of the real estate.
    * @return
    */
   float ComputeWorstAspectRatio(std::vector<TreeNode<VizNode>>& row,
      const TreeNode<VizNode>* additionalItem, const float lengthOfShortestSide)
   {
      std::uintmax_t sumOfAllArea = std::accumulate(std::begin(row), std::end(row),
         std::uintmax_t{0}, [] (const std::uintmax_t result, const TreeNode<VizNode>& node)
      {
         return result + node.GetData().m_file.m_size;
      });

      const std::uintmax_t additionalItemSize = additionalItem
         ? additionalItem->GetData().m_file.m_size : 0;

      sumOfAllArea += additionalItemSize;

      std::uintmax_t largestElement;
      if (additionalItem)
      {
         largestElement = row.front().GetData().m_file.m_size > additionalItemSize
            ? row.front().GetData().m_file.m_size : additionalItemSize;
      }
      else
      {
         largestElement = row.front().GetData().m_file.m_size;
      }

      std::uintmax_t smallestElement;
      if (additionalItem)
      {
         smallestElement = row.back().GetData().m_file.m_size < additionalItemSize
            ? row.back().GetData().m_file.m_size : additionalItemSize;
      }
      else
      {
         smallestElement = row.back().GetData().m_file.m_size;
      }

      const float lengthSquared = lengthOfShortestSide * lengthOfShortestSide;lengthSquared;
      const float areaSquared = sumOfAllArea * sumOfAllArea;

      return std::max((lengthSquared * largestElement) / (areaSquared),
         (areaSquared) / (lengthSquared * smallestElement));
   }

   void IterativeSquarify(TreeNode<VizNode>& children)
   {
      TreeNode<VizNode>* currentChild = &children;
      std::vector<TreeNode<VizNode>> currentRow;

      while (currentChild)
      {
         TreeNode<VizNode>* parentNode = &*currentChild->GetParent();
         assert(parentNode);

         const float shortestSide =
            std::min(parentNode->GetData().m_block.m_height, parentNode->GetData().m_block.m_depth);

         // If worst aspect ratio improves, add block to current row:
         if (ComputeWorstAspectRatio(currentRow, currentChild, shortestSide) <
             ComputeWorstAspectRatio(currentRow, nullptr, shortestSide))
         {
            currentRow.emplace_back(*currentChild);
         }
         else // if aspect ratio gets worse, create a new row:
         {
            LayoutStrip(currentRow, parentNode->GetData().m_block);

            currentRow.clear();
            currentRow.emplace_back(*currentChild);
         }

         currentChild = &*currentChild->GetNextSibling();
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
   auto& tree = m_diskScanner.GetDirectoryTree();

   std::for_each(std::begin(tree), std::end(tree),
      [] (TreeNode<VizNode>& node)
   {
      node.SortChildren([] (const TreeNode<VizNode>& lhs, const TreeNode<VizNode>& rhs)
         { return lhs.GetData().m_file.m_size < rhs.GetData().m_file.m_size; });
   });

   // Set the size of the root visualization:
   tree.GetHead()->GetData().m_block = Block(QVector3D(0, 0, 0), 10.0f, 0.125f, 10.0f);

   IterativeSquarify(*tree.GetHead()->GetFirstChild());
}
