#ifndef RAY_H
#define RAY_H

#include <QVector3D>

class Ray
{
  public:
    Ray(const QVector3D& origin, const QVector3D& direction);

    QVector3D Origin() const;

    QVector3D Direction() const;

  private:
    QVector3D m_origin;
    QVector3D m_direction;
};

#endif // RAY_H
