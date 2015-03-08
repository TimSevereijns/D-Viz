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
   void LayoutStrip(std::vector<TreeNode<VizNode>>& children, const Block& realEstate)
   {
      static const float BLOCK_HEIGHT = 0.0625f;
      static const float BLOCK_TO_REAL_ESTATE_RATIO = 0.9f;

      if (node.GetChildCount() == 0 || !realEstate.isValid())
      {
         return;
      }

      const size_t nodeCount = children.size();
      const float percentCovered = 0.0f;

      float additionalCoverage = 0.0f;

      std::for_each(std::begin(children), std::end(children),
         [&] (const TreeNode<VizNode>& node)
      {
         VizNode& data = node.GetData();
         const std::uintmax_t fileSize = data.m_file.m_size;

         if (fileSize == 0)
         {
            assert(!"Found a node without a size!");
            return;
         }

         const auto percentageOfParent = static_cast<float>(fileSize) /
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
               widthPaddingPerSide
               BLOCK_HEIGHT,
               -(realEstate.m_depth * percentCovered) - depthPaddingPerSide,
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
   float ComputeWorstAspectRatio(std::vector<TreeNode<VizNode>>& row)
   {
      const auto maxElement = std::max_element(std::begin(row), std::end(row),
         [] (const TreeNode<VizNode>& lhs, const TreeNode<VizNode>& rhs) -> bool
      {
         return true;
      });

      return 1.0f;
   }

   /**
    * @brief Squarify
    *
    * Pseudo:
    *
    * VizNode& currentNode = head(remainingChildren);
    *
    * if ((the worst aspect ratio of all blocks in the row without currentNode) <=
    *     (the worst aspect ratio of all blocks in the row with currentNode)) then
    * {
    *    recurse on the tail(remainingChildren) and the row with the currentNode in it.
    * }
    * else
    * {
    *    layout the current items in the row.
    *    recurse on the remaining area with all the children.
    * }
    *
    * @param children
    * @param realEstate
    * @param longestSide
    */
   void Squarify(TreeNode<VizNode>& children, Block& realEstate, float longestSide)
   {
      // Do some stuff...

      auto row = std::vector<TreeNode<VizNode>>();
      LayoutStrip(row, Block());
   }

   /**
    * @brief Squarify
    * @param node
    */
   void ParseNode(TreeNode<VizNode>& node)
   {
      Squarify(*node.GetFirstChild(), Block(), 10.0f);
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

