#include "Model/precisePoint.h"

PrecisePoint::PrecisePoint(double x, double y, double z) : m_x{ x }, m_y{ y }, m_z{ z }
{
}

double PrecisePoint::x() const noexcept
{
    return m_x;
}

double PrecisePoint::y() const noexcept
{
    return m_y;
}

double PrecisePoint::z() const noexcept
{
    return m_z;
}

float PrecisePoint::xAsFloat() const noexcept
{
    return static_cast<float>(m_x);
}

float PrecisePoint::yAsFloat() const noexcept
{
    return static_cast<float>(m_y);
}

float PrecisePoint::zAsFloat() const noexcept
{
    return static_cast<float>(m_z);
}
