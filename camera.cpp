#include "camera.h"

#include <QtMath>
#include <QMatrix>

Camera::Camera()
{
}

Camera::~Camera()
{
}

QMatrix3x3& Camera::GetPosition() const
{
   return m_position;
}

void Camera::SetPosition(const QMatrix3x3 newPosition)
{
   m_position = newPosition;
}

void Camera::OffsetPosition(QMatrix3x3 offset)
{

}

QMatrix3x3& Camera::GetOrientation() const
{
   return m_orientation;
}

void Camera::LookAt(const QMatrix3x3& position)
{

}

QMatrix3x3 Camera::Forward() const
{
   //QMatrix4x4 forward =
}

QMatrix3x3 Camera::Backward() const
{

}

QMatrix3x3 Camera::Right() const
{

}

QMatrix3x3 Camera::Left() const
{

}

QMatrix3x3 Camera::Up() const
{

}

QMatrix3x3 Camera::Down() const
{

}

QMatrix4x4 Camera::GetProjection() const
{

}

QMatrix4x4 Camera::GetView() const
{

}

