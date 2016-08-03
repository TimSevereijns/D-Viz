#ifndef DOUBLEPOINT3D_H
#define DOUBLEPOINT3D_H

/**
 * @brief The DoublePoint3D class provides the bare necessities needed to perform all node layout
 * arithmetic with double (instead of single) floating point precision.
 */
class PrecisePoint
{
   public:

      PrecisePoint() = default;

      PrecisePoint(
         double x,
         double y,
         double z);

      double x() const;
      double y() const;
      double z() const;

      friend inline auto operator+(
         const PrecisePoint& lhs,
         const PrecisePoint& rhs)
      {
         return PrecisePoint
         {
            lhs.m_x + rhs.m_x,
            lhs.m_y + rhs.m_y,
            lhs.m_z + rhs.m_z
         };
      }

   private:

      double m_x{ 0.0 };
      double m_y{ 0.0 };
      double m_z{ 0.0 };
};

#endif // DOUBLEPOINT3D_H