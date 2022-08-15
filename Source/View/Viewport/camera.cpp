#include "View/Viewport/camera.h"

#include "constants.h"

#include <cmath>

#include <gsl/assert>

namespace
{
    void NormalizeAngles(double& yaw, double& pitch) noexcept
    {
        yaw = std::fmod(yaw, 360.0);

        // Ensure all values are positive:
        if (yaw < 0.0) {
            yaw += 360.0;
        }

        constexpr auto maxVertical = 90.0;

        if (pitch > maxVertical) {
            pitch = maxVertical;
        } else if (pitch < -maxVertical) {
            pitch = -maxVertical;
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

void Camera::SetOrientation(double pitch, double yaw) noexcept
{
    m_yaw = yaw;
    m_pitch = pitch;

    NormalizeAngles(m_yaw, m_pitch);
}

QMatrix4x4 Camera::GetOrientation() const noexcept
{
    QMatrix4x4 orientation;
    orientation.rotate(static_cast<float>(m_pitch), 1, 0, 0);
    orientation.rotate(static_cast<float>(m_yaw), 0, 1, 0);

    return orientation;
}

void Camera::OffsetOrientation(double pitch, double yaw) noexcept
{
    m_yaw += yaw;
    m_pitch += pitch;

    NormalizeAngles(m_yaw, m_pitch);
}

void Camera::LookAt(const QVector3D& target) noexcept
{
    Expects(target != m_position);

    QVector3D direction = target - m_position;
    direction.normalize();

    const auto pitchInRadians = static_cast<double>(std::asin(-direction.y()));
    m_pitch = pitchInRadians * Constants::Math::RadiansToDegrees;

    const auto yawInRadians = static_cast<double>(std::atan2(direction.x(), -direction.z()));
    m_yaw = yawInRadians * Constants::Math::RadiansToDegrees;

    NormalizeAngles(m_yaw, m_pitch);
}

QVector3D Camera::Forward() const noexcept
{
    return QVector3D{ GetOrientation().inverted() * QVector4D{ 0, 0, -1, 1 } };
}

QVector3D Camera::Backward() const noexcept
{
    return -Forward();
}

QVector3D Camera::Right() const noexcept
{
    return QVector3D{ GetOrientation().inverted() * QVector4D{ 1, 0, 0, 1 } };
}

QVector3D Camera::Left() const noexcept
{
    return -Right();
}

QVector3D Camera::Up() const noexcept
{
    return QVector3D{ GetOrientation().inverted() * QVector4D{ 0, 1, 0, 1 } };
}

QVector3D Camera::Down() const noexcept
{
    return -Up();
}

QMatrix4x4 Camera::GetProjectionMatrix() const noexcept
{
    QMatrix4x4 matrix;
    matrix.perspective(m_fieldOfView, m_aspectRatio, m_nearPlane, m_farPlane);

    return matrix;
}

QMatrix4x4 Camera::GetViewMatrix() const noexcept
{
    QMatrix4x4 matrix = GetOrientation();
    matrix.translate(-m_position);

    return matrix;
}

QMatrix4x4 Camera::GetProjectionViewMatrix() const noexcept
{
    return GetProjectionMatrix() * GetViewMatrix();
}

QVector3D
Camera::Unproject(const QPoint& point, float viewDepth, const QMatrix4x4& modelMatrix) const
    noexcept
{
    const auto modelViewProjectionMatrix = GetProjectionMatrix() * GetViewMatrix() * modelMatrix;

    auto wasMatrixInvertible = false;
    const QMatrix4x4 inverseMatrix = modelViewProjectionMatrix.inverted(&wasMatrixInvertible);

    if (!wasMatrixInvertible) {
        Expects(false);
        return {};
    }

    const auto viewportWidth = static_cast<float>(m_viewport.width());
    const auto viewportHeight = static_cast<float>(m_viewport.height());

    const auto x = 2.0f * (point.x() - m_viewport.x()) / viewportWidth - 1;
    const auto y = 2.0f * (point.y() - m_viewport.y()) / viewportHeight - 1;
    const auto z = 2.0f * viewDepth - 1.0f;

    const QVector3D viewportPoint{ x, y, z };
    const QVector3D unprojectedPoint = inverseMatrix.map(viewportPoint);

    return unprojectedPoint;
}

QPoint Camera::MapToOpenGLViewport(const QPoint& widgetCoordinates) const noexcept
{
    const int invertedY = m_viewport.y() + (m_viewport.height() - widgetCoordinates.y());
    return { widgetCoordinates.x(), invertedY };
}

Ray Camera::ShootRayIntoScene(const QPoint& widgetCoordinates) const noexcept
{
    const QPoint glCoordinates = MapToOpenGLViewport(widgetCoordinates);

    const QVector3D nearPlanePoint = Unproject(glCoordinates, 0.0f, QMatrix4x4{});
    const QVector3D farPlanePoint = Unproject(glCoordinates, 1.0f, QMatrix4x4{});

    const auto direction = QVector3D(farPlanePoint - nearPlanePoint).normalized();

    return { nearPlanePoint, direction };
}

bool Camera::IsPointInFrontOfCamera(const QVector3D& point) const noexcept
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

void Camera::SetNearPlane(float nearPlane) noexcept
{
    m_nearPlane = nearPlane;
}

float Camera::GetNearPlane() const noexcept
{
    return m_nearPlane;
}

void Camera::SetFarPlane(float farPlane) noexcept
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
