#include "doublePoint3d.h"

DoublePoint3D::DoublePoint3D()
   : m_x(0.0),
     m_y(0.0),
     m_z(0.0)
{
}

DoublePoint3D::DoublePoint3D(double x, double y, double z)
   : m_x(x),
     m_y(y),
     m_z(z)
{
}

double DoublePoint3D::x() const
{
   return m_x;
}

double DoublePoint3D::y() const
{
   return m_y;
}

double DoublePoint3D::z() const
{
   return m_z;
}

