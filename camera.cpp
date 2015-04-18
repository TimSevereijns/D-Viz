#include "camera.h"

#include <assert.h>
#include <iostream>
#include <math.h>

// Keep vertical angle less than 90 degrees to avoid gimbal lock:
static const double MAX_VERTICAL_ANGLE = 85.0;

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
     m_horizontalAngle(0),
     m_position(0, 0, 0),
     m_aspectRatio(1.0f),
     m_nearPlane(1.0f),
     m_farPlane(2000.0f),
     m_fieldOfView(45.0f)
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

void Camera::OffsetPosition(const QVector3D& offset)
{
   m_position += offset;
}

QMatrix4x4 Camera::GetOrientation() const
{
   QMatrix4x4 orientation;
   orientation.rotate(m_verticalAngle, 1, 0, 0);
   orientation.rotate(m_horizontalAngle, 0, 1, 0);
   return orientation;
}

void Camera::OffsetOrientation(float verticalAngle, float horizontalAngle)
{
   m_horizontalAngle += horizontalAngle;
   m_verticalAngle += verticalAngle;

   NormalizeAngles(m_horizontalAngle, m_verticalAngle);
}

void Camera::LookAt(const QVector3D& position)
{
   assert(position != m_position);
   QVector3D direction = position - m_position;
   direction.normalize();

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
   QMatrix4x4 matrix;
   matrix.perspective(m_fieldOfView, m_aspectRatio, m_nearPlane, m_farPlane);

   return matrix;
}

QMatrix4x4 Camera::GetView() const
{
   QMatrix4x4 matrix = GetOrientation();
   matrix.translate(-m_position);
   return matrix;
}

QMatrix4x4 Camera::GetMatrix() const
{
   return GetProjection() * GetView();
}

void Camera::SetAspectRatio(const float ratio)
{
   m_aspectRatio = ratio;
}

void Camera::SetFieldOfView(const float angle)
{
   if (angle > 85.0f)
   {
      m_fieldOfView = 85.0f;
   }
   else if (angle < 5.0f)
   {
      m_fieldOfView = 5.0f;
   }
   else
   {
      m_fieldOfView = angle;
   }
}

float Camera::GetFieldOfView() const
{
   return m_fieldOfView;
}

void Camera::IncreaseFieldOfView()
{
   m_fieldOfView += 5.0f;

   if (m_fieldOfView > 85.0f)
   {
      m_fieldOfView = 85.0f;
   }
}

void Camera::DecreaseFieldOfView()
{
   m_fieldOfView -= 5.0f;

   if (m_fieldOfView < 5.0f)
   {
      m_fieldOfView = 5.0f;
   }
}
