#include "DataStructs/precisePoint.h"

PrecisePoint::PrecisePoint(double x, double y, double z) : m_x{ x }, m_y{ y }, m_z{ z }
{
}

double PrecisePoint::x() const
{
    return m_x;
}

double PrecisePoint::y() const
{
    return m_y;
}

double PrecisePoint::z() const
{
    return m_z;
}

float PrecisePoint::xAsFloat() const
{
    return static_cast<float>(m_x);
}

float PrecisePoint::yAsFloat() const
{
    return static_cast<float>(m_y);
}

float PrecisePoint::zAsFloat() const
{
    return static_cast<float>(m_z);
}
