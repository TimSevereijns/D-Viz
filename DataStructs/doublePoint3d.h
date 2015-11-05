#ifndef DOUBLEPOINT3D_H
#define DOUBLEPOINT3D_H

/**
 * @brief The DoublePoint3D class provides the bare necessities needed to perform all node layout
 * arithmetic with double (instead of single) floating point precision.
 */
class DoublePoint3D
{
   public:
      DoublePoint3D();
      DoublePoint3D(double x, double y, double z);

      double x() const;
      double y() const;
      double z() const;

      friend inline DoublePoint3D operator+(const DoublePoint3D& lhs, const DoublePoint3D& rhs)
      {
         return DoublePoint3D
         {
            lhs.m_x + rhs.m_x,
            lhs.m_y + rhs.m_y,
            lhs.m_z + rhs.m_z
         };
      }

   private:
      double m_x;
      double m_y;
      double m_z;
};

#endif // DOUBLEPOINT3D_H
