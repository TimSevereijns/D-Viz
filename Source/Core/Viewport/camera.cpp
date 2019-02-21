#include "camera.h"

#include "../constants.h"

#include <cmath>
#include <iostream>

#include <gsl/gsl_assert>

namespace
{
    void NormalizeAngles(double& horizontalAngle, double& verticalAngle) noexcept
    {
        // Calculate floating point remainder:
        horizontalAngle = std::fmod(horizontalAngle, 360.0);

        // Ensure all values are positive:
        if (horizontalAngle < 0.0) {
            horizontalAngle += 360.0;
        }

        const double maxVertical{ 90.0 };

        if (verticalAngle > maxVertical) {
            verticalAngle = maxVertical;
        } else if (verticalAngle < -maxVertical) {
            verticalAngle = -maxVertical;
        }
    }
} // namespace

const QVector3D& Camera::GetPosition() const noexcept
{
    return m_position;
}

void Camera::SetPosition(const QVector3D& newPosition) noexcept
{
    m_position = newPosition;
}

void Camera::OffsetPosition(const QVector3D& offset) noexcept
{
    m_position += offset;
}

void Camera::SetOrientation(double pitch, double yaw)
{
    m_horizontalAngle = yaw;
    m_verticalAngle = pitch;

    NormalizeAngles(m_horizontalAngle, m_verticalAngle);
}

QMatrix4x4 Camera::GetOrientation() const
{
    QMatrix4x4 orientation;
    orientation.rotate(static_cast<float>(m_verticalAngle), 1, 0, 0);
    orientation.rotate(static_cast<float>(m_horizontalAngle), 0, 1, 0);
    return orientation;
}

void Camera::OffsetOrientation(double pitch, double yaw) noexcept
{
    m_horizontalAngle += yaw;
    m_verticalAngle += pitch;

    NormalizeAngles(m_horizontalAngle, m_verticalAngle);
}

void Camera::LookAt(const QVector3D& point)
{
    Expects(point != m_position);

    QVector3D direction = point - m_position;
    direction.normalize();

    m_verticalAngle =
        static_cast<double>(std::asin(-direction.y())) * Constants::Math::RADIANS_TO_DEGREES;

    m_horizontalAngle = static_cast<double>(std::atan2(direction.x(), -direction.z())) *
                        Constants::Math::RADIANS_TO_DEGREES;

    NormalizeAngles(m_horizontalAngle, m_verticalAngle);
}

QVector3D Camera::Forward() const
{
    const QVector4D forwardVector = GetOrientation().inverted() * QVector4D{ 0, 0, -1, 1 };
    return QVector3D{ forwardVector };
}

QVector3D Camera::Backward() const
{
    return -Forward();
}

QVector3D Camera::Right() const
{
    const QVector4D rightVector = GetOrientation().inverted() * QVector4D{ 1, 0, 0, 1 };
    return QVector3D{ rightVector };
}

QVector3D Camera::Left() const
{
    return -Right();
}

QVector3D Camera::Up() const
{
    const QVector4D upVector = GetOrientation().inverted() * QVector4D{ 0, 1, 0, 1 };
    return QVector3D{ upVector };
}

QVector3D Camera::Down() const
{
    return -Up();
}

QMatrix4x4 Camera::GetProjectionMatrix() const
{
    QMatrix4x4 matrix;
    matrix.perspective(m_fieldOfView, m_aspectRatio, m_nearPlane, m_farPlane);

    return matrix;
}

QMatrix4x4 Camera::GetViewMatrix() const
{
    QMatrix4x4 matrix = GetOrientation();
    matrix.translate(-m_position);
    return matrix;
}

QMatrix4x4 Camera::GetProjectionViewMatrix() const
{
    return GetProjectionMatrix() * GetViewMatrix();
}

QVector3D
Camera::Unproject(const QPoint& point, float viewDepth, const QMatrix4x4& modelMatrix) const
{
    const auto modelViewProjectionMatrix = GetProjectionMatrix() * GetViewMatrix() * modelMatrix;

    bool wasMatrixInvertible = false;
    const QMatrix4x4 inverseMatrix = modelViewProjectionMatrix.inverted(&wasMatrixInvertible);

    if (!wasMatrixInvertible) {
        Expects(false);
        return {};
    }

    const float x =
        2.0f * (point.x() - m_viewport.x()) / static_cast<float>(m_viewport.width()) - 1;
    const float y =
        2.0f * (point.y() - m_viewport.y()) / static_cast<float>(m_viewport.height()) - 1;
    const float z = 2.0f * viewDepth - 1.0f;

    const QVector3D viewportPoint{ x, y, z };
    const QVector3D unprojectedPoint = inverseMatrix.map(viewportPoint);

    return unprojectedPoint;
}

QPoint Camera::MapToOpenGLViewport(const QPoint& widgetCoordinates) const
{
    const int invertedY = m_viewport.y() + (m_viewport.height() - widgetCoordinates.y());
    return { widgetCoordinates.x(), invertedY };
}

Ray Camera::ShootRayIntoScene(const QPoint& widgetCoordinates) const
{
    const QPoint glCoordinates = MapToOpenGLViewport(widgetCoordinates);

    const QVector3D nearPlanePoint = Unproject(glCoordinates, 0.0f, QMatrix4x4{});
    const QVector3D farPlanePoint = Unproject(glCoordinates, 1.0f, QMatrix4x4{});

    const auto direction = QVector3D(farPlanePoint - nearPlanePoint).normalized();

    return Ray{ nearPlanePoint, direction };
}

bool Camera::IsPointInFrontOfCamera(const QVector3D& point) const
{
    const QVector3D distanceToPoint = m_position - point;

    const auto inverseRotationMatrix = GetOrientation().inverted();
    const auto result = distanceToPoint * inverseRotationMatrix;

    return result.z() > 0;
}

void Camera::SetViewport(const QRect& size) noexcept
{
    m_viewport = size;
    m_aspectRatio = static_cast<float>(size.width()) / static_cast<float>(size.height());
}

void Camera::SetNearPlane(float nearPlane)
{
    m_nearPlane = nearPlane;
}

float Camera::GetNearPlane() const noexcept
{
    return m_nearPlane;
}

void Camera::SetFarPlane(float farPlane)
{
    m_farPlane = farPlane;
}

float Camera::GetFarPlane() const noexcept
{
    return m_farPlane;
}

float Camera::GetAspectRatio() const noexcept
{
    return m_aspectRatio;
}

QRect Camera::GetViewport() const noexcept
{
    return m_viewport;
}

void Camera::SetFieldOfView(int angle) noexcept
{
    if (angle > 85) {
        m_fieldOfView = 85;
    } else if (angle < 5) {
        m_fieldOfView = 5;
    } else {
        m_fieldOfView = angle;
    }
}

int Camera::GetVerticalFieldOfView() const noexcept
{
    return m_fieldOfView;
}

void Camera::IncreaseFieldOfView() noexcept
{
    m_fieldOfView += 5;

    if (m_fieldOfView > 85) {
        m_fieldOfView = 85;
    }
}

void Camera::DecreaseFieldOfView() noexcept
{
    m_fieldOfView -= 5;

    if (m_fieldOfView < 5) {
        m_fieldOfView = 5;
    }
}
