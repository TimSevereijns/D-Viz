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
    * @brief RowSizeInBytes
    * @param row
    * @return
    */
   std::uintmax_t RowSizeInBytes(std::vector<TreeNode<VizNode>*>& row)
   {
      std::uintmax_t sumOfFileSizes = std::accumulate(std::begin(row), std::end(row),
         std::uintmax_t{0}, [] (const std::uintmax_t result, const TreeNode<VizNode>* node)
      {
         return result + node->GetData().m_file.m_size;
      });

      return sumOfFileSizes;
   }

   /**
    * @brief LayoutStrip slices the real estate into file size proportional pieces.
    * @param[in] row                The items to be placed in the current piece of real estate.
    * @param[in] realEsate          The space into which the nodes have to be placed.
    */
   void LayoutStrip(std::vector<TreeNode<VizNode>*>& row, Block& realEstate)
   {
      static const float BLOCK_HEIGHT = 0.0625f;
      static const float BLOCK_TO_REAL_ESTATE_RATIO = 0.9f;

      if (row.size() == 0 || !realEstate.IsDefined())
      {
         return;
      }

      const size_t nodeCount = row.size();
      const float parentFileSize = row.front()->GetParent()->GetData().m_file.m_size;

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
               static_cast<float>(parentFileSize);

         if (realEstate.m_width > realEstate.m_depth)
         {
            const auto paddedBlockWidth = realEstate.m_width * percentageOfParent;
            const auto actualBlockWidth = paddedBlockWidth * BLOCK_TO_REAL_ESTATE_RATIO;
            const auto widthPaddingPerSide = ((realEstate.m_width * 0.1f) / nodeCount) / 2.0f;

            const auto actualBlockDepth = realEstate.m_depth * BLOCK_TO_REAL_ESTATE_RATIO;
            const auto depthPaddingPerSide = (realEstate.m_depth - actualBlockDepth) / 2.0f;

            const auto offset = QVector3D(
               (realEstate.m_width * realEstate.m_percentCovered) + widthPaddingPerSide,
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
            const auto widthPaddingPerSide = (realEstate.m_width - actualBlockWidth) / 2.0f;

            const auto offset = QVector3D(
               widthPaddingPerSide,
               BLOCK_HEIGHT,
               -(realEstate.m_depth * realEstate.m_percentCovered) - depthPaddingPerSide
            );

            data.m_block = Block(realEstate.m_vertices.front() + offset,
               actualBlockWidth,
               BLOCK_HEIGHT,
               actualBlockDepth
            );

            additionalCoverage = (actualBlockDepth + (2 * depthPaddingPerSide)) /
                  realEstate.m_depth;
         }

         assert(additionalCoverage > 0.0f && additionalCoverage <= 1.0f);
         assert(data.m_block.IsDefined());
         //assert(realEstate.m_percentCovered <= 1.0f);

         realEstate.m_percentCovered += additionalCoverage;
      }
   }

   /**
    * @brief ComputeWorstAspectRatio
    * @param[in] row                   The nodes that have been placed in the current real estate.
    * @param[in] additionalItem        One additional item item to be considered in the same estate.
    * @param[in] lengthOfShortestSide  Length of shortest side of the real estate.
    * @return
    */
   float ComputeWorstAspectRatio(std::vector<TreeNode<VizNode>*>& row,
      const TreeNode<VizNode>* additionalItem, const float shortestSideOfRow)
   {
      if (row.empty() && !additionalItem)
      {
         std::cout << "Both row and additional item are empty!" << std::endl;
         return std::numeric_limits<float>::max();
      }

      std::uintmax_t sumOfFileSizes = RowSizeInBytes(row);

      const std::uintmax_t additionalItemSize = additionalItem
         ? additionalItem->GetData().m_file.m_size : 0;

      sumOfFileSizes += additionalItemSize;

      std::uintmax_t largestElement;
      if (additionalItem && !row.empty())
      {
         largestElement = row.front()->GetData().m_file.m_size > additionalItemSize
            ? row.front()->GetData().m_file.m_size : additionalItemSize;
      }
      else if (additionalItem)
      {
         largestElement = additionalItemSize;
      }
      else
      {
         largestElement = row.front()->GetData().m_file.m_size;
      }

      std::uintmax_t smallestElement;
      if (additionalItem && !row.empty())
      {
         smallestElement = row.back()->GetData().m_file.m_size < additionalItemSize
            ? row.back()->GetData().m_file.m_size : additionalItemSize;
      }
      else if (additionalItem)
      {
         smallestElement = additionalItemSize;
      }
      else
      {
         smallestElement = row.back()->GetData().m_file.m_size;
      }

      const float lengthSquared = shortestSideOfRow * shortestSideOfRow;
      const float areaSquared = sumOfFileSizes * sumOfFileSizes;

      const float worstRatio = std::max((lengthSquared * largestElement) / (areaSquared),
         (areaSquared) / (lengthSquared * smallestElement));

      std::cout << "Ratio: " << worstRatio << std::endl;

      return worstRatio;
   }

   /**
    * @brief IterativeSquarify
    * @param children
    */
   void IterativeSquarify(TreeNode<VizNode>& children)
   {
      TreeNode<VizNode>* currentChild = &children;
      TreeNode<VizNode>* firstChild = currentChild;
      std::vector<TreeNode<VizNode>*> currentRow;

      while (currentChild)
      {
         TreeNode<VizNode>* parentNode = &*currentChild->GetParent();
         assert(parentNode);

         const float shortestSide = std::min(parentNode->GetData().m_block.m_width,
                                             parentNode->GetData().m_block.m_depth);

         // If worst aspect ratio improves, add block to current row:
         if (ComputeWorstAspectRatio(currentRow, currentChild, shortestSide) <=
             ComputeWorstAspectRatio(currentRow, nullptr,      shortestSide))
         {
            currentRow.emplace_back(currentChild);
         }
         else // if aspect ratio gets worse, layout current row and create a new row:
         {
            PrintRow(currentRow);
            LayoutStrip(currentRow, parentNode->GetData().m_block);

            currentRow.clear();
            currentRow.emplace_back(currentChild);
         }

         currentChild = &*currentChild->GetNextSibling();
      }

      currentChild = firstChild;

      if (!currentRow.empty())
      {
         PrintRow(currentRow);
         LayoutStrip(currentRow, currentChild->GetParent()->GetData().m_block);
      }

      while (currentChild)
      {
         IterativeSquarify(*currentChild->GetFirstChild());
         currentChild = &*currentChild->GetPreviousSibling();
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
   //auto& tree = m_diskScanner.GetDirectoryTree();

   FileInfo fileInfo{L"Dummy Root Node", 24, FILE_TYPE::DIRECTORY};
   VizNode rootNode{fileInfo, Block{QVector3D(0.0f, 0.0f, 0.0f), 6.0f, 0.025f, 4.0f}};

   // TODO: Create tree containing the same data as in the paper:
   Tree<VizNode> tree(rootNode);
   FileInfo dummyInfo6{L"6", 6, FILE_TYPE::REGULAR};
   tree.GetHead()->AppendChild(VizNode(dummyInfo6));
   tree.GetHead()->AppendChild(VizNode(dummyInfo6));

   FileInfo dummyInfo4{L"4", 4, FILE_TYPE::REGULAR};
   tree.GetHead()->AppendChild(VizNode(dummyInfo4));

   FileInfo dummyInfo3{L"3", 3, FILE_TYPE::REGULAR};
   tree.GetHead()->AppendChild(VizNode(dummyInfo3));

   FileInfo dummyInfo2{L"2", 2, FILE_TYPE::REGULAR};
   tree.GetHead()->AppendChild(VizNode(dummyInfo2));
   tree.GetHead()->AppendChild(VizNode(dummyInfo2));

   FileInfo dummyInfo1{L"1", 1, FILE_TYPE::REGULAR};
   tree.GetHead()->AppendChild(VizNode(dummyInfo1));

   for (TreeNode<VizNode>& node : tree)
   {
      node.SortChildren([] (const TreeNode<VizNode>& lhs, const TreeNode<VizNode>& rhs)
      {
         return lhs.GetData().m_file.m_size >= rhs.GetData().m_file.m_size;
      });
   }

   // Set the size of the root visualization:
//   tree.GetHead()->GetFirstChild()->GetData().m_block =
//         Block(QVector3D(0, 0, 0), 10.0f, 0.0625f, 10.0f);

   IterativeSquarify(*tree.GetHead()->GetFirstChild());

   m_hasDataBeenParsed = true;

   m_diskScanner.GetDirectoryTree() = tree;
}
