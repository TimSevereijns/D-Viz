#ifndef PRECISEPOINT_H
#define PRECISEPOINT_H

/**
 * @brief Provides the bare necessities needed to represent three-dimensional point with double
 * precision.
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

#endif // PRECISEPOINT_H
