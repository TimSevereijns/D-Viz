#ifndef CAMERA_H
#define CAMERA_H

#include <QMatrix3x3>
#include <QMatrix4x4>

class Camera
{
   public:
      Camera();
      ~Camera();

      QVector3D& GetPosition();
      void SetPosition(const QVector3D& newPosition);

      QMatrix4x4 GetOrientation() const;

      void LookAt(const QVector3D& position);

      QVector3D Forward() const;
      QVector3D Backward() const;
      QVector3D Right() const;
      QVector3D Left() const;
      QVector3D Up() const;
      QVector3D Down() const;

      QMatrix4x4 GetProjection() const;

      QMatrix4x4 GetView() const;

   private:
      QVector3D m_position;

      QMatrix4x4 m_projection;

      double m_horizontalAngle;
      double m_verticalAngle;
};

#endif // CAMERA_H
