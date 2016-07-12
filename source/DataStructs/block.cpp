#include "block.h"

Block::Block(
   const DoublePoint3D& origin,
   const double width,
   const double height,
   const double depth,
   const bool generateVertices /* = false */)
   :
   m_width{ width },
   m_height{ height },
   m_depth{ depth },
   m_percentCovered{ 0.0 },
   m_origin{ origin },
   m_nextRowOrigin{ origin.x(), origin.y() + height, origin.z() }
{
   if (!generateVertices)
   {
      return;
   }

   const auto x = static_cast<float>(origin.x());
   const auto y = static_cast<float>(origin.y());
   const auto z = static_cast<float>(origin.z());

   m_vertices.reserve(VERTICES_PER_BLOCK * 2);
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

DoublePoint3D Block::ComputeNextChildOrigin() const
{
   return m_origin + DoublePoint3D{ 0, m_height, 0 };
}

double Block::GetWidth() const
{
   return m_width;
}

double Block::GetHeight() const
{
   return m_height;
}

double Block::GetDepth() const
{
   return m_depth;
}

DoublePoint3D Block::GetOrigin() const
{
   return m_origin;
}

DoublePoint3D Block::GetNextRowOrigin() const
{
   return m_nextRowOrigin;
}

void Block::SetNextRowOrigin(const DoublePoint3D& origin)
{
   m_nextRowOrigin = origin;
}

double Block::GetCoverage() const
{
   return m_percentCovered;
}

void Block::IncreaseCoverageBy(double additionalCoverage)
{
   m_percentCovered += additionalCoverage;
}

const QVector<QVector3D>& Block::GetVertices() const
{
   return m_vertices;
}
