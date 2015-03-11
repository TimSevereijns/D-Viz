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
   void LayoutStrip(std::vector<TreeNode<VizNode>>& row, RealEstate& realEstate)
   {
      static const float BLOCK_HEIGHT = 0.0625f;
      static const float BLOCK_TO_REAL_ESTATE_RATIO = 0.9f;

      if (row.size() == 0 || !realEstate.m_block.IsDefined())
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

         if (realEstate.m_block.m_width > realEstate.m_block.m_depth)
         {
            const auto paddedBlockWidth = realEstate.m_block.m_width * percentageOfParent;
            const auto actualBlockWidth = paddedBlockWidth * BLOCK_TO_REAL_ESTATE_RATIO;
            const auto widthPaddingPerSide = ((realEstate.m_block.m_width * 0.1f) / nodeCount) / 2.0f;

            const auto actualBlockDepth = realEstate.m_block.m_depth * BLOCK_TO_REAL_ESTATE_RATIO;
            const auto depthPaddingPerSide = (realEstate.m_block.m_height - actualBlockDepth) / 2.0f;

            const auto offset = QVector3D(
               (realEstate.m_block.m_width * percentCovered) + widthPaddingPerSide,
               BLOCK_HEIGHT,
               -depthPaddingPerSide
            );

            data.m_block = Block(realEstate.m_block.m_vertices[0] + offset,
               actualBlockWidth,
               BLOCK_HEIGHT,
               actualBlockDepth
            );

            additionalCoverage = (actualBlockWidth + (2 * widthPaddingPerSide)) /
                  realEstate.m_block.m_width;
         }
         else
         {
            const auto paddedBlockDepth = realEstate.m_block.m_depth * percentageOfParent;
            const auto actualBlockDepth = paddedBlockDepth * BLOCK_TO_REAL_ESTATE_RATIO;
            const auto depthPaddingPerSide = ((realEstate.m_block.m_depth * 0.1f) / nodeCount) / 2.0f;

            const auto actualBlockWidth = realEstate.m_block.m_width * BLOCK_TO_REAL_ESTATE_RATIO;
            const auto widthPaddingPerSide = (realEstate.m_block.m_height - actualBlockWidth) / 2.0f;

            const auto offset = QVector3D(
               widthPaddingPerSide,
               BLOCK_HEIGHT,
               -(realEstate.m_block.m_depth * percentCovered) - depthPaddingPerSide
            );

            data.m_block = Block(realEstate.m_block.m_vertices[0] + offset,
               actualBlockWidth,
               BLOCK_HEIGHT,
               actualBlockDepth
            );

            additionalCoverage = (actualBlockDepth + (2 * depthPaddingPerSide)) /
                  realEstate.m_block.m_depth;
         }

         if (additionalCoverage)
         {
            assert(!"The new block does not appear to have required dimensions!");
         }

         realEstate.m_block.m_percentCovered += additionalCoverage;
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

   /**
    * @brief Squarify is the main recursive function that drives the creation of the Squarifed map.
    * @param[in] children           The remaining child nodes that need to be laid out.
    * @param[in] realEstate         The space available into which the children can be laid out.
    */
   void Squarify(TreeNode<VizNode>& children, std::vector<TreeNode<VizNode>>& row,
      RealEstate& realEstate)
   {
      TreeNode<VizNode>& firstChild = children;

      const float shortestSide = std::min(realEstate.m_block.m_height, realEstate.m_block.m_depth);

      // If worst aspect ratio improves, add block to current row:
      if (ComputeWorstAspectRatio(row, nullptr, shortestSide) <=
          ComputeWorstAspectRatio(row, &firstChild, shortestSide))
      {
         RealEstate remainingRealEstate;
         // TODO: Compute remaining real estate.

         row.emplace_back(firstChild);
         Squarify(*children.GetNextSibling(), row, remainingRealEstate);
      }
      else // if aspect ratio gets worse, create a new row:
      {
         LayoutStrip(row, realEstate);

         RealEstate freshRealEstate;
         // TODO: Compute new real estate.

         Squarify(*children.GetNextSibling(), std::vector<TreeNode<VizNode>>(), freshRealEstate);
      }
   }

   /**
    * @brief ParseNode kicks off the squarification of each node.
    * @param node[in]               The current node being parsed.
    */
   void ParseNode(TreeNode<VizNode>& node, DiskScanner& scanner)
   {
      Block parentBlock = node.GetParent()
         ? node.GetParent()->GetData().m_block
         : Block(QVector3D(0, 0, 0), 10.0f, 0.125f, 10.0f); // The dummy base node.

      RealEstate realEstate(parentBlock, scanner.GetDirectoryTree().GetHead()->GetData().m_file.m_size);

      Squarify(*node.GetFirstChild(), std::vector<TreeNode<VizNode>>(), realEstate);
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

   //std::for_each(tree.beginPreOrder(), tree.endPreOrder(), ParseNode);
}
