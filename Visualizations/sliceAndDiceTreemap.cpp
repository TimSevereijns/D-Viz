#include "sliceAndDiceTreemap.h"

#include <iostream>

namespace
{
   void ParseNode(TreeNode<VizNode>& node)
   {
      static const float BLOCK_HEIGHT = 0.0625f;

      VizNode& data = node.GetData();
      const std::uintmax_t fileSize = data.m_file.m_size;

      if (fileSize == 0 || !node.GetParent())
      {
         return;
      }

      const std::uintmax_t parentSize = node.GetParent()->GetData().m_file.m_size;
      const float percentageOfParent = static_cast<float>(fileSize) / static_cast<float>(parentSize);

      Block& parentBlock = node.GetParent()->GetData().m_block;
      const unsigned int siblingCount = node.GetParent()->GetChildCount() + 1;

      float additionalCoverage = 0.0f;

      // Slice perpendicular to the X-axis:
      if (parentBlock.m_width > parentBlock.m_depth)
      {
         const auto paddedBlockWidth = parentBlock.m_width * percentageOfParent;
         const auto actualBlockWidth = paddedBlockWidth * 0.9f;
         const auto widthPaddingPerSide = ((parentBlock.m_width * 0.1f) / siblingCount) / 2.0f;

         const auto actualBlockDepth = parentBlock.m_depth * 0.9f;
         const auto depthPaddingPerSide = (parentBlock.m_depth - actualBlockDepth) / 2.0f;

         const auto offset = QVector3D(
            (parentBlock.m_width * parentBlock.m_percentCovered) + widthPaddingPerSide,   // X
            parentBlock.m_height,                                                         // Y
            -depthPaddingPerSide                                                          // Z
         );

         data.m_block = Block(parentBlock.m_vertices[0] + offset,                         // Origin
            actualBlockWidth,                                                             // Width
            BLOCK_HEIGHT,                                                                 // Height
            actualBlockDepth                                                              // Depth
         );

         additionalCoverage = (actualBlockWidth + (2 * widthPaddingPerSide)) / parentBlock.m_width;
      }
      else
      {
         const auto paddedBlockDepth = parentBlock.m_depth * percentageOfParent;
         const auto actualBlockDepth = paddedBlockDepth * 0.9f;
         const auto depthPaddingPerSide = ((parentBlock.m_depth * 0.1f) / siblingCount) / 2.0f;

         const auto actualBlockWidth = parentBlock.m_width * 0.9f;
         const auto widthPaddingPerSide = (parentBlock.m_width - actualBlockWidth) / 2.0f;

         const auto offset = QVector3D(
            widthPaddingPerSide,                                                          // X
            parentBlock.m_height,                                                         // Y
            -(parentBlock.m_depth * parentBlock.m_percentCovered) - depthPaddingPerSide   // Z
         );

         data.m_block = Block(parentBlock.m_vertices[0] + offset,                         // Origin
            actualBlockWidth,                                                             // Width
            BLOCK_HEIGHT,                                                                 // Height
            actualBlockDepth                                                              // Depth
         );

         additionalCoverage = (actualBlockDepth + (2 * depthPaddingPerSide)) / parentBlock.m_depth;
       }

      assert(additionalCoverage != 0.0f);
      parentBlock.m_percentCovered += additionalCoverage;

      assert(data.m_block.IsDefined());
   }
}

SliceAndDiceTreeMap::SliceAndDiceTreeMap(const std::wstring& rawPath)
   : Visualization(rawPath)
{
}

SliceAndDiceTreeMap::~SliceAndDiceTreeMap()
{
}

void SliceAndDiceTreeMap::ParseScan()
{
   auto& tree = m_diskScanner.GetDirectoryTree();

   std::cout << "Sorting raw tree..." << std::endl;

   std::for_each(std::begin(tree), std::end(tree),
      [] (TreeNode<VizNode>& node)
   {
      node.SortChildren([] (const TreeNode<VizNode>& lhs, const TreeNode<VizNode>& rhs)
         { return lhs.GetData().m_file.m_size < rhs.GetData().m_file.m_size; });
   });

   std::cout << "Parsing raw tree..." << std::endl;

   std::for_each(tree.beginPreOrder(), tree.endPreOrder(), ParseNode);

   m_hasDataBeenParsed = true;
}

