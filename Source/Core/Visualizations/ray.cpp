#include "Visualizations/ray.h"

Ray::Ray(const QVector3D& origin, const QVector3D& direction)
    : m_origin{ origin }, m_direction{ direction }
{
}

QVector3D Ray::Origin() const
{
    return m_origin;
}

QVector3D Ray::Direction() const
{
    return m_direction;
}
