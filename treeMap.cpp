#include "treeMap.h"

#include <algorithm>
#include <iostream>

TreeMap::TreeMap()
{
}

TreeMap::TreeMap(const std::wstring& rawRootNodePath)
   : m_diskScanner(rawRootNodePath)
{
   std::atomic<std::pair<std::uintmax_t, bool>> progress{std::make_pair(0, false)};
   m_diskScanner.ScanInNewThread(&progress);

   while (progress.load().second == false)
   {
      std::cout << "Files scanned so far: "
                << progress.load().first << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(1));
   }

   m_diskScanner.JoinScanningThread();
   m_diskScanner.PrintTreeMetadata();
}

TreeMap::~TreeMap()
{
}

Tree<VizNode>& TreeMap::ParseDirectoryTree()
{
   std::cout << "Parsing raw tree..." << std::endl;

   auto& tree = m_diskScanner.GetDirectoryTree();

   std::for_each(++tree.beginPreOrder(), tree.endPreOrder(),
      [&] (TreeNode<VizNode>& node)
   {
      VizNode& data = node.GetData();
      const std::uintmax_t fileSize = data.m_file.m_size;

      if (fileSize == 0 || !node.GetParent())
      {
         return;
      }

      const std::uintmax_t parentSize = node.GetParent()->GetData().m_file.m_size;
      const float percentageOfParent = static_cast<float>(fileSize) / static_cast<float>(parentSize);

      Block& parentBlock = node.GetParent()->GetData().m_block;

      // Calculate the appropriate padding for the new block based on space available on top of
      // the parent block:
      const auto paddedBlockWidth = parentBlock.m_width * percentageOfParent;
      const auto actualBlockWidth = paddedBlockWidth * 0.9f;
      const auto widthPaddingPerSide = (paddedBlockWidth - actualBlockWidth) / 2.0f;

      const auto actualBlockDepth = parentBlock.m_depth * 0.9f;
      const auto depthPaddingPerSide = (parentBlock.m_depth - actualBlockDepth) / 2.0f;

      const auto offset = QVector3D(
         (parentBlock.m_width * parentBlock.m_percentCovered) + widthPaddingPerSide,      // X
         parentBlock.m_height,                                                            // Y
         -depthPaddingPerSide);                                                           // Z

      data.m_block = Block(parentBlock.m_vertices[0] + offset,                            // Corner
         actualBlockWidth,                                                                // Width
         0.125f,                                                                          // Height
         actualBlockDepth);                                                               // Depth

      parentBlock.m_percentCovered += percentageOfParent;

      assert(data.m_block.IsDefined());
   });

   return tree;
}

QVector<QVector3D> TreeMap::CreateBlockVertices(const QVector3D &bottomLeft, const float width,
                                                const float height, const float depth)
{
   const float x = bottomLeft.x();
   const float y = bottomLeft.y();
   const float z = bottomLeft.z();

   QVector<QVector3D> blockVertices;
   blockVertices.reserve(72);
   blockVertices
      // Front:                                               // Vertex Normals:
      << QVector3D(x           , y            , z           ) << QVector3D( 0,  0,  1)
      << QVector3D(x + width   , y            , z           ) << QVector3D( 0,  0,  1)
      << QVector3D(x           , y + height   , z           ) << QVector3D( 0,  0,  1)
      << QVector3D(x + width   , y + height   , z           ) << QVector3D( 0,  0,  1)
      << QVector3D(x           , y + height   , z           ) << QVector3D( 0,  0,  1)
      << QVector3D(x + width   , y            , z           ) << QVector3D( 0,  0,  1)
      // Right:
      << QVector3D(x + width   , y            , z           ) << QVector3D( 1,  0,  0)
      << QVector3D(x + width   , y            , z - depth   ) << QVector3D( 1,  0,  0)
      << QVector3D(x + width   , y + height   , z           ) << QVector3D( 1,  0,  0)
      << QVector3D(x + width   , y + height   , z - depth   ) << QVector3D( 1,  0,  0)
      << QVector3D(x + width   , y + height   , z           ) << QVector3D( 1,  0,  0)
      << QVector3D(x + width   , y            , z - depth   ) << QVector3D( 1,  0,  0)
      // Back:
      << QVector3D(x + width   , y            , z - depth   ) << QVector3D( 0,  0, -1)
      << QVector3D(x           , y            , z - depth   ) << QVector3D( 0,  0, -1)
      << QVector3D(x + width   , y + height   , z - depth   ) << QVector3D( 0,  0, -1)
      << QVector3D(x           , y + height   , z - depth   ) << QVector3D( 0,  0, -1)
      << QVector3D(x + width   , y + height   , z - depth   ) << QVector3D( 0,  0, -1)
      << QVector3D(x           , y            , z - depth   ) << QVector3D( 0,  0, -1)
      // Left:
      << QVector3D(x           , y            , z - depth   ) << QVector3D(-1,  0,  0)
      << QVector3D(x           , y            , z           ) << QVector3D(-1,  0,  0)
      << QVector3D(x           , y + height   , z - depth   ) << QVector3D(-1,  0,  0)
      << QVector3D(x           , y + height   , z           ) << QVector3D(-1,  0,  0)
      << QVector3D(x           , y + height   , z - depth   ) << QVector3D(-1,  0,  0)
      << QVector3D(x           , y            , z           ) << QVector3D(-1,  0,  0)
      // Bottom:
      << QVector3D(x           , y            , z - depth   ) << QVector3D( 0, -1,  0)
      << QVector3D(x + width   , y            , z - depth   ) << QVector3D( 0, -1,  0)
      << QVector3D(x           , y            , z           ) << QVector3D( 0, -1,  0)
      << QVector3D(x + width   , y            , z           ) << QVector3D( 0, -1,  0)
      << QVector3D(x           , y            , z           ) << QVector3D( 0, -1,  0)
      << QVector3D(x + width   , y            , z - depth   ) << QVector3D( 0, -1,  0)
      // Top:
      << QVector3D(x           , y + height   , z           ) << QVector3D( 0,  1,  0)
      << QVector3D(x + width   , y + height   , z           ) << QVector3D( 0,  1,  0)
      << QVector3D(x           , y + height   , z - depth   ) << QVector3D( 0,  1,  0)
      << QVector3D(x + width   , y + height   , z - depth   ) << QVector3D( 0,  1,  0)
      << QVector3D(x           , y + height   , z - depth   ) << QVector3D( 0,  1,  0)
      << QVector3D(x + width   , y + height   , z           ) << QVector3D( 0,  1,  0);

   return blockVertices;
}

QVector<QVector3D> TreeMap::CreateBlockColors()
{
   QVector<QVector3D> blockColors;
   blockColors.reserve(36);
   blockColors
      << QVector3D(1, 0, 0) << QVector3D(1, 0, 0) << QVector3D(1, 0, 0) // Front
      << QVector3D(1, 0, 0) << QVector3D(1, 0, 0) << QVector3D(1, 0, 0)
      << QVector3D(0, 1, 0) << QVector3D(0, 1, 0) << QVector3D(0, 1, 0) // Right
      << QVector3D(0, 1, 0) << QVector3D(0, 1, 0) << QVector3D(0, 1, 0)
      << QVector3D(1, 0, 0) << QVector3D(1, 0, 0) << QVector3D(1, 0, 0) // Back
      << QVector3D(1, 0, 0) << QVector3D(1, 0, 0) << QVector3D(1, 0, 0)
      << QVector3D(0, 1, 0) << QVector3D(0, 1, 0) << QVector3D(0, 1, 0) // Left
      << QVector3D(0, 1, 0) << QVector3D(0, 1, 0) << QVector3D(0, 1, 0)
      << QVector3D(0, 0, 1) << QVector3D(0, 0, 1) << QVector3D(0, 0, 1) // Top
      << QVector3D(0, 0, 1) << QVector3D(0, 0, 1) << QVector3D(0, 0, 1)
      << QVector3D(0, 0, 1) << QVector3D(0, 0, 1) << QVector3D(0, 0, 1) // Bottom
      << QVector3D(0, 0, 1) << QVector3D(0, 0, 1) << QVector3D(0, 0, 1);

   return blockColors;
}

QVector<QVector3D> TreeMap::CreateDirectoryColors()
{
   QVector<QVector3D> blockColors;
   blockColors.reserve(36);
   blockColors
      << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) // Front
      << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) << QVector3D(1, 1, 1)
      << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) // Right
      << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) << QVector3D(1, 1, 1)
      << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) // Back
      << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) << QVector3D(1, 1, 1)
      << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) // Left
      << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) << QVector3D(1, 1, 1)
      << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) // Top
      << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) << QVector3D(1, 1, 1)
      << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) // Bottom
      << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) << QVector3D(1, 1, 1);

   return blockColors;
}
