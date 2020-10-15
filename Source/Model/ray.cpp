#include "Model/ray.h"

Ray::Ray(const QVector3D& origin, const QVector3D& direction) noexcept
    : m_origin{ origin }, m_direction{ direction }
{
}

const QVector3D& Ray::Origin() const noexcept
{
    return m_origin;
}

const QVector3D& Ray::Direction() const noexcept
{
    return m_direction;
}
