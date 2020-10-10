#include "Model/ray.h"

Ray::Ray(const QVector3D& origin, const QVector3D& direction) noexcept
    : m_origin{ origin }, m_direction{ direction }
{
}

QVector3D Ray::Origin() const noexcept
{
    return m_origin;
}

QVector3D Ray::Direction() const noexcept
{
    return m_direction;
}
