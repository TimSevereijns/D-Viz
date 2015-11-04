#include "block.h"

Block::Block()
   : width(0.0),
     height(0.0),
     depth(0.0),
     percentCovered(0.0)
{
}

Block::Block(const DoublePoint3D& origin, const double width, const double height,
             const double depth)
   : width(width),
     height(height),
     depth(depth),
     percentCovered(0.0),
     origin(origin),
     nextRowOrigin(origin.x(), origin.y() + height, origin.z())
{
   const auto x = static_cast<float>(origin.x());
   const auto y = static_cast<float>(origin.y());
   const auto z = static_cast<float>(origin.z());

   QVector<QVector3D> frontFace;
   frontFace.reserve(12);
   frontFace
      // Front:                                               // Vertex Normals:        // Index:
      << QVector3D(x           , y            , z           ) << QVector3D( 0,  0,  1)  // 0
      << QVector3D(x + width   , y            , z           ) << QVector3D( 0,  0,  1)  // 2
      << QVector3D(x           , y + height   , z           ) << QVector3D( 0,  0,  1)  // 4
      << QVector3D(x + width   , y + height   , z           ) << QVector3D( 0,  0,  1)  // 6
      << QVector3D(x           , y + height   , z           ) << QVector3D( 0,  0,  1)  // 8
      << QVector3D(x + width   , y            , z           ) << QVector3D( 0,  0,  1); // 10

   QVector<QVector3D> rightFace;
   rightFace.reserve(12);
   rightFace
      // Right:
      << QVector3D(x + width   , y            , z           ) << QVector3D( 1,  0,  0)  // 12
      << QVector3D(x + width   , y            , z - depth   ) << QVector3D( 1,  0,  0)  // 14
      << QVector3D(x + width   , y + height   , z           ) << QVector3D( 1,  0,  0)  // 16
      << QVector3D(x + width   , y + height   , z - depth   ) << QVector3D( 1,  0,  0)  // 18
      << QVector3D(x + width   , y + height   , z           ) << QVector3D( 1,  0,  0)  // 20
      << QVector3D(x + width   , y            , z - depth   ) << QVector3D( 1,  0,  0); // 22

   QVector<QVector3D> backFace;
   backFace.reserve(12);
   backFace
      // Back:
      << QVector3D(x + width   , y            , z - depth   ) << QVector3D( 0,  0, -1)  // 24
      << QVector3D(x           , y            , z - depth   ) << QVector3D( 0,  0, -1)  // 26
      << QVector3D(x + width   , y + height   , z - depth   ) << QVector3D( 0,  0, -1)  // 28
      << QVector3D(x           , y + height   , z - depth   ) << QVector3D( 0,  0, -1)  // 30
      << QVector3D(x + width   , y + height   , z - depth   ) << QVector3D( 0,  0, -1)  // 32
      << QVector3D(x           , y            , z - depth   ) << QVector3D( 0,  0, -1); // 34

   QVector<QVector3D> leftFace;
   leftFace.reserve(12);
   leftFace
      // Left:
      << QVector3D(x           , y            , z - depth   ) << QVector3D(-1,  0,  0)  // 36
      << QVector3D(x           , y            , z           ) << QVector3D(-1,  0,  0)  // 38
      << QVector3D(x           , y + height   , z - depth   ) << QVector3D(-1,  0,  0)  // 40
      << QVector3D(x           , y + height   , z           ) << QVector3D(-1,  0,  0)  // 42
      << QVector3D(x           , y + height   , z - depth   ) << QVector3D(-1,  0,  0)  // 44
      << QVector3D(x           , y            , z           ) << QVector3D(-1,  0,  0); // 46

   QVector<QVector3D> topFace;
   topFace.reserve(12);
   topFace
      // Top:
      << QVector3D(x           , y + height   , z           ) << QVector3D( 0,  1,  0)  // 48
      << QVector3D(x + width   , y + height   , z           ) << QVector3D( 0,  1,  0)  // 50
      << QVector3D(x           , y + height   , z - depth   ) << QVector3D( 0,  1,  0)  // 52
      << QVector3D(x + width   , y + height   , z - depth   ) << QVector3D( 0,  1,  0)  // 54
      << QVector3D(x           , y + height   , z - depth   ) << QVector3D( 0,  1,  0)  // 56
      << QVector3D(x + width   , y + height   , z           ) << QVector3D( 0,  1,  0); // 58

   blockFaces.reserve(5);
   blockFaces
      << BlockFace(frontFace, BlockFace::Side::FRONT)
      << BlockFace(rightFace, BlockFace::Side::RIGHT)
      << BlockFace(backFace,  BlockFace::Side::BACK)
      << BlockFace(leftFace,  BlockFace::Side::LEFT)
      << BlockFace(topFace,   BlockFace::Side::TOP);
}

Block::FaceIterator Block::begin() const
{
   Block::FaceIterator iterator(this);
   return iterator;
}

Block::FaceIterator Block::end() const
{
   Block::FaceIterator iterator(this, Block::FACES_PER_BLOCK);
   return iterator;
}

bool Block::HasVolume() const
{
   return (width != 0.0 && height != 0.0 && depth != 0.0);
}

bool Block::IsNotInverted() const
{
   // The indices used are keyed off of the vertex order used in the constructor above.
   //return m_vertices[36].x() < m_vertices[12].x();

   //return m_blockFaces[3].m_vertices[0].x() < m_blockFaces[0].m_vertices[0].x();
   return true;
}

DoublePoint3D Block::GetNextChildOrigin() const
{
   return origin + DoublePoint3D(0, height, 0);
}
