#include "treeMap.h"

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

QVector<QVector3D> TreeMap::CreateBlockColors() const
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
