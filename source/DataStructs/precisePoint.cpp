#include "precisePoint.h"

PrecisePoint::PrecisePoint(
   double x,
   double y,
   double z)
   :
   m_x{ x },
   m_y{ y },
   m_z{ z }
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

