#include "camera.h"

#include <assert.h>
#include <math.h>

// Keep vertical angle less than 90 degrees to avoid gimbal lock:
static const double MAX_VERTICAL_ANGLE = 85.0f;

namespace
{
   void NormalizeAngles(double& horizontalAngle, double& verticalAngle)
   {
      // Calculate floating point remainder:
      horizontalAngle = std::fmod(horizontalAngle, 360.0f);

      // Ensure all values are positive:
      if (horizontalAngle < 0.0f)
      {
         horizontalAngle += 360.0f;
      }

      if (verticalAngle > MAX_VERTICAL_ANGLE)
      {
         verticalAngle = MAX_VERTICAL_ANGLE;
      }
      else if (verticalAngle < -MAX_VERTICAL_ANGLE)
      {
         verticalAngle = -MAX_VERTICAL_ANGLE;
      }
   }
}

Camera::Camera()
   : m_verticalAngle(0),
     m_horizontalAngle(0)
{
}

Camera::~Camera()
{
}

QVector3D& Camera::GetPosition()
{
   return m_position;
}

void Camera::SetPosition(const QVector3D& newPosition)
{
   m_position = newPosition;
}

QMatrix4x4 Camera::GetOrientation() const
{
   QMatrix4x4 orientation;
   orientation.rotate(m_verticalAngle, 1, 0, 0);
   orientation.rotate(m_horizontalAngle, 0, 1, 0);
   return orientation;
}

void Camera::LookAt(const QVector3D& position)
{
   assert(position != m_position);
   QVector3D direction = position - m_position;

   m_verticalAngle = std::asin(-direction.y());
   m_horizontalAngle = std::atan2(-direction.x(), -direction.z());

   NormalizeAngles(m_horizontalAngle, m_verticalAngle);
}

QVector3D Camera::Forward() const
{
   const QVector4D forwardVector = GetOrientation().inverted() * QVector4D(0, 0, -1, 1);
   return QVector3D(forwardVector);
}

QVector3D Camera::Backward() const
{
   return -Forward();
}

QVector3D Camera::Right() const
{
   const QVector4D rightVector = GetOrientation().inverted() * QVector4D(1, 0, 0, 1);
   return QVector3D(rightVector);
}

QVector3D Camera::Left() const
{
   return -Right();
}

QVector3D Camera::Up() const
{
   const QVector4D upVector = GetOrientation().inverted() * QVector4D(0, 1, 0, 1);
   return QVector3D(upVector);
}

QVector3D Camera::Down() const
{
   return -Up();
}

QMatrix4x4 Camera::GetProjection() const
{
   // TODO: Implement

   return QMatrix4x4();
}

QMatrix4x4 Camera::GetView() const
{
   return QMatrix4x4();
}
