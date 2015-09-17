#include "block.h"

Block::Block()
   : m_width(0.0),
     m_height(0.0),
     m_depth(0.0),
     m_percentCovered(0.0)
{
}

Block::Block(const DoublePoint3D& bottomLeft, const double width, const double height,
             const double depth)
   : m_width(width),
     m_height(height),
     m_depth(depth),
     m_percentCovered(0.0),
     m_blockOrigin(bottomLeft),
     m_nextRowOrigin(bottomLeft.x(), bottomLeft.y() + height, bottomLeft.z())
{
   const auto x = static_cast<float>(bottomLeft.x());
   const auto y = static_cast<float>(bottomLeft.y());
   const auto z = static_cast<float>(bottomLeft.z());

   m_vertices.reserve(VERTICES_PER_BLOCK);
   m_vertices
      // Front:                                               // Vertex Normals:        // Index:
      << QVector3D(x           , y            , z           ) << QVector3D( 0,  0,  1)  // 0
      << QVector3D(x + width   , y            , z           ) << QVector3D( 0,  0,  1)  // 2
      << QVector3D(x           , y + height   , z           ) << QVector3D( 0,  0,  1)  // 4
      << QVector3D(x + width   , y + height   , z           ) << QVector3D( 0,  0,  1)  // 6
      << QVector3D(x           , y + height   , z           ) << QVector3D( 0,  0,  1)  // 8
      << QVector3D(x + width   , y            , z           ) << QVector3D( 0,  0,  1)  // 10
      // Right:
      << QVector3D(x + width   , y            , z           ) << QVector3D( 1,  0,  0)  // 12
      << QVector3D(x + width   , y            , z - depth   ) << QVector3D( 1,  0,  0)  // 14
      << QVector3D(x + width   , y + height   , z           ) << QVector3D( 1,  0,  0)  // 16
      << QVector3D(x + width   , y + height   , z - depth   ) << QVector3D( 1,  0,  0)  // 18
      << QVector3D(x + width   , y + height   , z           ) << QVector3D( 1,  0,  0)  // 20
      << QVector3D(x + width   , y            , z - depth   ) << QVector3D( 1,  0,  0)  // 22
      // Back:
      << QVector3D(x + width   , y            , z - depth   ) << QVector3D( 0,  0, -1)  // 24
      << QVector3D(x           , y            , z - depth   ) << QVector3D( 0,  0, -1)  // 26
      << QVector3D(x + width   , y + height   , z - depth   ) << QVector3D( 0,  0, -1)  // 28
      << QVector3D(x           , y + height   , z - depth   ) << QVector3D( 0,  0, -1)  // 30
      << QVector3D(x + width   , y + height   , z - depth   ) << QVector3D( 0,  0, -1)  // 32
      << QVector3D(x           , y            , z - depth   ) << QVector3D( 0,  0, -1)  // 34
      // Left:
      << QVector3D(x           , y            , z - depth   ) << QVector3D(-1,  0,  0)  // 36
      << QVector3D(x           , y            , z           ) << QVector3D(-1,  0,  0)  // 38
      << QVector3D(x           , y + height   , z - depth   ) << QVector3D(-1,  0,  0)  // 40
      << QVector3D(x           , y + height   , z           ) << QVector3D(-1,  0,  0)  // 42
      << QVector3D(x           , y + height   , z - depth   ) << QVector3D(-1,  0,  0)  // 44
      << QVector3D(x           , y            , z           ) << QVector3D(-1,  0,  0)  // 46
      // Top:
      << QVector3D(x           , y + height   , z           ) << QVector3D( 0,  1,  0)  // 48
      << QVector3D(x + width   , y + height   , z           ) << QVector3D( 0,  1,  0)  // 50
      << QVector3D(x           , y + height   , z - depth   ) << QVector3D( 0,  1,  0)  // 52
      << QVector3D(x + width   , y + height   , z - depth   ) << QVector3D( 0,  1,  0)  // 54
      << QVector3D(x           , y + height   , z - depth   ) << QVector3D( 0,  1,  0)  // 56
      << QVector3D(x + width   , y + height   , z           ) << QVector3D( 0,  1,  0); // 58
}

bool Block::HasVolume() const
{
   return (m_width != 0.0 && m_height != 0.0 && m_depth != 0.0);
}

bool Block::IsNotInverted() const
{
   // The indices used are keyed off of the vertex order used in the constructor above.
   return m_vertices[36].x() < m_vertices[12].x();
}

DoublePoint3D Block::GetNextChildOrigin() const
{
   return m_blockOrigin + DoublePoint3D(0, m_height, 0);
}
