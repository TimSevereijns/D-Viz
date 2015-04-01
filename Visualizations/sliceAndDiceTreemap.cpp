#include "sliceAndDiceTreemap.h"

#include <iostream>

namespace
{
   /**
    * @brief ParseNode parses the current tree node and creates the visualizations of the current
    * node's children on top of the current node's visualized block.
    * @param[in] node               The current node to visualize.
    */
   void ParseNode(TreeNode<VizNode>& node)
   {
      VizNode& data = node.GetData();
      const std::uintmax_t fileSize = data.m_file.m_size;

      if (fileSize == 0 || !node.GetParent())
      {
         return;
      }

      const std::uintmax_t parentSize = node.GetParent()->GetData().m_file.m_size;
      const auto percentageOfParent = static_cast<float>(fileSize) / static_cast<float>(parentSize);

      Block& parentBlock = node.GetParent()->GetData().m_block;
      const unsigned int siblingCount = node.GetParent()->GetChildCount() + 1;

      float additionalCoverage = 0.0f;

      if (parentBlock.m_width > parentBlock.m_depth)          // Slice perpendicular to the X-axis:
      {
         const auto paddedBlockWidth = parentBlock.m_width * percentageOfParent;
         const auto actualBlockWidth = paddedBlockWidth * Visualization::PADDING_RATIO;
         const auto widthPaddingPerSide = ((parentBlock.m_width * 0.1f) / siblingCount) / 2.0f;

         const auto actualBlockDepth = parentBlock.m_depth * Visualization::PADDING_RATIO;
         const auto depthPaddingPerSide = (parentBlock.m_depth - actualBlockDepth) / 2.0f;

         const auto offset = QVector3D(
            (parentBlock.m_width * parentBlock.m_percentCovered) + widthPaddingPerSide,   // X
            parentBlock.m_height,                                                         // Y
            -depthPaddingPerSide                                                          // Z
         );

         data.m_block = Block(parentBlock.m_vertices[0] + offset,                         // Origin
            actualBlockWidth,                                                             // Width
            Visualization::BLOCK_HEIGHT,                                                  // Height
            actualBlockDepth                                                              // Depth
         );

         additionalCoverage = (actualBlockWidth + (2 * widthPaddingPerSide)) / parentBlock.m_width;
      }
      else                                                    // Slice perpendicular to the Y-axis:
      {
         const auto paddedBlockDepth = parentBlock.m_depth * percentageOfParent;
         const auto actualBlockDepth = paddedBlockDepth * Visualization::PADDING_RATIO;
         const auto depthPaddingPerSide = ((parentBlock.m_depth * 0.1f) / siblingCount) / 2.0f;

         const auto actualBlockWidth = parentBlock.m_width * Visualization::PADDING_RATIO;
         const auto widthPaddingPerSide = (parentBlock.m_width - actualBlockWidth) / 2.0f;

         const auto offset = QVector3D(
            widthPaddingPerSide,                                                          // X
            parentBlock.m_height,                                                         // Y
            -(parentBlock.m_depth * parentBlock.m_percentCovered) - depthPaddingPerSide   // Z
         );

         data.m_block = Block(parentBlock.m_vertices[0] + offset,                         // Origin
            actualBlockWidth,                                                             // Width
            Visualization::BLOCK_HEIGHT,                                                  // Height
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

   const auto startSortTime = std::chrono::high_resolution_clock::now();

   std::for_each(std::begin(tree), std::end(tree),
      [] (TreeNode<VizNode>& node)
   {
      node.SortChildren([] (const TreeNode<VizNode>& lhs, const TreeNode<VizNode>& rhs)
         { return lhs.GetData().m_file.m_size < rhs.GetData().m_file.m_size; });
   });

   const auto endSortTime = std::chrono::high_resolution_clock::now();

   std::chrono::duration<double> sortingTime =
      std::chrono::duration_cast<std::chrono::duration<double>>(endSortTime - startSortTime);

   std::cout << "Sort time (in seconds): " << sortingTime.count() << std::endl;

   const auto startParseTime = std::chrono::high_resolution_clock::now();
   std::for_each(tree.beginPreOrder(), tree.endPreOrder(), ParseNode);
   const auto endParseTime = std::chrono::high_resolution_clock::now();

   std::chrono::duration<double> parsingTime =
      std::chrono::duration_cast<std::chrono::duration<double>>(endParseTime - startParseTime);

   std::cout << "Parse time (in seconds): " << parsingTime.count() << std::endl;

   m_hasDataBeenParsed = true;
}

