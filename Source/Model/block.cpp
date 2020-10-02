#include "Model/block.h"

Block::Block(
    const PrecisePoint& origin, double blockWidth, double blockHeight, double blockDepth,
    bool generateVertices /* = false */) noexcept
    : m_origin{ origin },
      m_nextRowOrigin{ origin.x(), origin.y() + blockHeight, origin.z() },
      m_percentCovered{ 0.0 },
      m_width{ blockWidth },
      m_height{ blockHeight },
      m_depth{ blockDepth }
{
    if (!generateVertices) {
        return;
    }

    const auto x = origin.xAsFloat();
    const auto y = origin.yAsFloat();
    const auto z = origin.zAsFloat();

    const auto width = static_cast<float>(blockWidth);
    const auto height = static_cast<float>(blockHeight);
    const auto depth = static_cast<float>(blockDepth);

    // clang-format off
    m_vertices.reserve(VerticesPerBlock * 2);
    m_vertices
        // Front:                                        // Vertex Normals:               // Index
        << QVector3D{ x, y, z }                          << QVector3D{ 0.0f, 0.0f, 1.0f } // 0
        << QVector3D{ x + width, y, z }                  << QVector3D{ 0.0f, 0.0f, 1.0f } // 2
        << QVector3D{ x, y + height, z }                 << QVector3D{ 0.0f, 0.0f, 1.0f } // 4
        << QVector3D{ x + width, y + height, z }         << QVector3D{ 0.0f, 0.0f, 1.0f } // 6
        << QVector3D{ x, y + height, z }                 << QVector3D{ 0.0f, 0.0f, 1.0f } // 8
        << QVector3D{ x + width, y, z }                  << QVector3D{ 0.0f, 0.0f, 1.0f } // 10

        // Right:
        << QVector3D{ x + width, y, z }                  << QVector3D{ 1.0f, 0.0f, 0.0f } // 12
        << QVector3D{ x + width, y, z - depth }          << QVector3D{ 1.0f, 0.0f, 0.0f } // 14
        << QVector3D{ x + width, y + height, z }         << QVector3D{ 1.0f, 0.0f, 0.0f } // 16
        << QVector3D{ x + width, y + height, z - depth } << QVector3D{ 1.0f, 0.0f, 0.0f } // 18
        << QVector3D{ x + width, y + height, z }         << QVector3D{ 1.0f, 0.0f, 0.0f } // 20
        << QVector3D{ x + width, y, z - depth }          << QVector3D{ 1.0f, 0.0f, 0.0f } // 22

        // Back:
        << QVector3D{ x + width, y, z - depth }          << QVector3D{ 0.0f, 0.0f, -1.0f } // 24
        << QVector3D{ x, y, z - depth }                  << QVector3D{ 0.0f, 0.0f, -1.0f } // 26
        << QVector3D{ x + width, y + height, z - depth } << QVector3D{ 0.0f, 0.0f, -1.0f } // 28
        << QVector3D{ x, y + height, z - depth }         << QVector3D{ 0.0f, 0.0f, -1.0f } // 30
        << QVector3D{ x + width, y + height, z - depth } << QVector3D{ 0.0f, 0.0f, -1.0f } // 32
        << QVector3D{ x, y, z - depth }                  << QVector3D{ 0.0f, 0.0f, -1.0f } // 34

        // Left:
        << QVector3D{ x, y, z - depth }                  << QVector3D{ -1.0f, 0.0f, 0.0f } // 36
        << QVector3D{ x, y, z }                          << QVector3D{ -1.0f, 0.0f, 0.0f } // 38
        << QVector3D{ x, y + height, z - depth }         << QVector3D{ -1.0f, 0.0f, 0.0f } // 40
        << QVector3D{ x, y + height, z }                 << QVector3D{ -1.0f, 0.0f, 0.0f } // 42
        << QVector3D{ x, y + height, z - depth }         << QVector3D{ -1.0f, 0.0f, 0.0f } // 44
        << QVector3D{ x, y, z }                          << QVector3D{ -1.0f, 0.0f, 0.0f } // 46

        // Top:
        << QVector3D{ x, y + height, z }                 << QVector3D{ 0.0f, 1.0f, 0.0f }  // 48
        << QVector3D{ x + width, y + height, z }         << QVector3D{ 0.0f, 1.0f, 0.0f }  // 50
        << QVector3D{ x, y + height, z - depth }         << QVector3D{ 0.0f, 1.0f, 0.0f }  // 52
        << QVector3D{ x + width, y + height, z - depth } << QVector3D{ 0.0f, 1.0f, 0.0f }  // 54
        << QVector3D{ x, y + height, z - depth }         << QVector3D{ 0.0f, 1.0f, 0.0f }  // 56
        << QVector3D{ x + width, y + height, z }         << QVector3D{ 0.0f, 1.0f, 0.0f }; // 58
    // clang-format on
}

bool Block::HasVolume() const noexcept
{
    return (m_width != 0.0 && m_height != 0.0 && m_depth != 0.0);
}

PrecisePoint Block::ComputeNextChildOrigin() const noexcept
{
    return m_origin + PrecisePoint{ 0, m_height, 0 };
}

double Block::GetWidth() const noexcept
{
    return m_width;
}

double Block::GetHeight() const noexcept
{
    return m_height;
}

double Block::GetDepth() const noexcept
{
    return m_depth;
}

PrecisePoint Block::GetOrigin() const noexcept
{
    return m_origin;
}

PrecisePoint Block::GetNextRowOrigin() const noexcept
{
    return m_nextRowOrigin;
}

void Block::SetNextRowOrigin(const PrecisePoint& origin) noexcept
{
    m_nextRowOrigin = origin;
}

double Block::GetCoverage() const noexcept
{
    return m_percentCovered;
}

void Block::IncreaseCoverageBy(double additionalCoverage) noexcept
{
    m_percentCovered += additionalCoverage;
}

const QVector<QVector3D>& Block::GetVerticesAndNormals() const noexcept
{
    return m_vertices;
}
