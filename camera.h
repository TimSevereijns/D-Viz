#ifndef CAMERA_H
#define CAMERA_H

#include <QMatrix3x3>
#include <QMatrix4x4>

class Camera
{
   public:
      Camera();
      ~Camera();

      QMatrix3x3& GetPosition() const;
      void SetPosition(const QMatrix3x3 newPosition);

      void OffsetPosition(QMatrix3x3 offset);

      QMatrix3x3& GetOrientation() const;

      void LookAt(const QMatrix3x3& position);

      QMatrix3x3 Forward() const;
      QMatrix3x3 Backward() const;
      QMatrix3x3 Right() const;
      QMatrix3x3 Left() const;
      QMatrix3x3 Up() const;
      QMatrix3x3 Down() const;

      QMatrix4x4 GetProjection() const;

      QMatrix4x4 GetView() const;

   private:
      QMatrix3x3 m_position;
      QMatrix4x4 m_projection;

      double m_horizontalAngle;
      double m_verticalAngle;
};

#endif // CAMERA_H
