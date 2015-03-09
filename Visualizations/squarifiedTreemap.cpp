#include "squarifiedTreemap.h"

#include <assert.h>
#include <iostream>

namespace
{
   /**
    * @brief ParseNode
    * @param node
    * @param realEsate
    * @param elevation
    * @param parentFileSize
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

            additionalCoverage = (actualBlockWidth + (2 * widthPaddingPerSide)) / realEstate.m_width;
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

            additionalCoverage = (actualBlockDepth + (2 * depthPaddingPerSide)) / realEstate.m_depth;
         }

         realEstate.m_percentCovered += additionalCoverage;
      });
   }

   /**
    * @brief ComputeWorstAspectRatio
    * @param row
    * @return
    */
   float ComputeWorstAspectRatio(std::vector<TreeNode<VizNode>>& row,
      TreeNode<VizNode>& additionalItem, float shortestWidth)
   {
      const auto maxElement = std::max_element(std::begin(row), std::end(row),
         [] (const TreeNode<VizNode>& lhs, const TreeNode<VizNode>& rhs) -> bool
      {
         const float lhsRatio = lhs.GetData().m_block.m_width / lhs.GetData().m_block.m_depth;
         const float rhsRatio = rhs.GetData().m_block.m_width / rhs.GetData().m_block.m_depth;

         return lhsRatio < rhsRatio;
      });

      return maxElement->GetData().m_block.m_width / maxElement->GetData().m_block.m_depth;
   }

   /**
    * @brief Squarify
    * @param children
    * @param realEstate
    * @param longestSide
    */
   void Squarify(TreeNode<VizNode>& children, std::vector<TreeNode<VizNode>>& row,
                 Block& realEstate, float shortestSide)
   {
      TreeNode<VizNode>& firstChild = children;
      if (ComputeWorstAspectRatio(row, TreeNode<VizNode>(), shortestSide) <=
          ComputeWorstAspectRatio(row, firstChild,          shortestSide))
      {
         Block remainingRealEstate;
         // TODO: Compute shortest remaining side.

         float shortestRemainingSide = remainingRealEstate.m_width < remainingRealEstate.m_depth
               ? remainingRealEstate.m_width
               : remainingRealEstate.m_depth;

         row.emplace_back(firstChild);

         Squarify(*children.GetNextSibling(), row, remainingRealEstate,
            shortestRemainingSide);
      }
      else
      {
         LayoutStrip(row, realEstate);

         Block newRealEstate;
         // TODO: Compute new real estate.

         float shortestRemainingSide = newRealEstate.m_width < newRealEstate.m_depth
               ? newRealEstate.m_width
               : newRealEstate.m_depth;

         Squarify(*children.GetNextSibling(), std::vector<TreeNode<VizNode>>(), newRealEstate,
            shortestRemainingSide);
      }
   }

   /**
    * @brief Squarify
    * @param node
    */
   void ParseNode(TreeNode<VizNode>& node)
   {
      Block parentBlock = node.GetParent()
         ? node.GetParent()->GetData().m_block
         : Block(QVector3D(0, 0, 0), 10.0f, 0.125f, 10.0f); // The dummy base node.

      Squarify(*node.GetFirstChild(), std::vector<TreeNode<VizNode>>(), parentBlock,
         parentBlock.m_width < parentBlock.m_depth ? parentBlock.m_width : parentBlock.m_depth);
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

   std::for_each(tree.beginPreOrder(), tree.endPreOrder(), ParseNode);
}

