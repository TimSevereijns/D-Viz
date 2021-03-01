#ifndef RAY_H
#define RAY_H

#include <QVector3D>

class Ray
{
  public:
    Ray(const QVector3D& origin, const QVector3D& direction) noexcept;

    /**
     * @returns The coordinates of the ray's origin.
     */
    const QVector3D& Origin() const noexcept;

    /**
     * @returns The direction vector emanating from the origin.
     */
    const QVector3D& Direction() const noexcept;

  private:
    QVector3D m_origin;
    QVector3D m_direction;
};

#endif // RAY_H
