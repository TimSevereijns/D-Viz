#ifndef CAMERA_H
#define CAMERA_H

#include <QMatrix3x3>
#include <QMatrix4x4>
#include <QRect>

#include "Visualizations/ray.h"

/**
 * @brief The Camera class represents the camera through which the scene is observed.
 */
class Camera
{
  public:
    /**
     * @brief Retrieves the camera's current position within 3D space.
     *
     * @returns The position of the camera within the scene.
     */
    const QVector3D& GetPosition() const noexcept;

    /**
     * @brief Sets the position of the camera in the scene.
     *
     * @param[in] newPosition     The new location of the camera.
     */
    void SetPosition(const QVector3D& newPosition) noexcept;

    /**
     * @brief Offsets the position of the camera by the specified amount relative to the current
     * position of the camera.
     *
     * @param[in] offset          The relative offset to apply.
     */
    void OffsetPosition(const QVector3D& offset) noexcept;

    /**
     * @brief Sets the current camera orientation in terms of pitch and yaw.
     *
     * There is currently no roll support.
     *
     * @param[in] pitch           In degrees.
     * @param[in] yaw             In degrees.
     */
    void SetOrientation(double pitch, double yaw) noexcept;

    /**
     * @brief Retrieves the camera's current orientation matrix.
     *
     * @returns The current orientation of the camera; that is, pitch, roll, and yaw.
     */
    QMatrix4x4 GetOrientation() const noexcept;

    /**
     * @brief Offset the current camera orientation in terms of pitch and yaw.
     *
     * There is currently no roll support.
     *
     * @param[in] pitch           In degrees.
     * @param[in] yaw             In degrees.
     */
    void OffsetOrientation(double pitch, double yaw) noexcept;

    /**
     * @brief Will orient the camera so that the specified point is within view.
     *
     * @param[in] point           The point to be brought into view.
     */
    void LookAt(const QVector3D& point) noexcept;

    /**
     * @brief Retrieves a vector pointing forward from the camera.
     *
     * @returns A vector that points forward relative to the current camera's position and
     * orientation.
     */
    QVector3D Forward() const noexcept;

    /**
     * @brief Retrieves a vector pointing backswards from the camera.
     *
     * @returns A vector that points backwards relative to the current camera's position and
     * orientation.
     */
    QVector3D Backward() const noexcept;

    /**
     * @brief Retrieves a vector pointing to the right of the camera.
     *
     * @returns A vector that points to the right relative to the current camera's position and
     * orientation.
     */
    QVector3D Right() const noexcept;

    /**
     * @brief Retrieves a vector pointing to the left of the camera.
     *
     * @returns A vector that points to the left relative to the current camera's position and
     * orientation.
     */
    QVector3D Left() const noexcept;

    /**
     * @brief Retrieves a vector pointing up from the camera.
     *
     * @returns A vector that points up relative to the current camera's position and orientation.
     */
    QVector3D Up() const noexcept;

    /**
     * @brief Retrieves a vector pointing down from the camera.
     *
     * @returns A vector that points down relative to the current camera's position and
     * orientation.
     */
    QVector3D Down() const noexcept;

    /**
     * @brief Retrieves the camera's current projection matrix.
     *
     * @returns The current projection matrix.
     */
    QMatrix4x4 GetProjectionMatrix() const noexcept;

    /**
     * @brief Retrieves the camera's current view matrix.
     *
     * @returns The current view matrix.
     */
    QMatrix4x4 GetViewMatrix() const noexcept;

    /**
     * @brief Retrieves the camera's projection-view matrix.
     *
     * @returns The projection matrix multiplied by the view matrix.
     */
    QMatrix4x4 GetProjectionViewMatrix() const noexcept;

    /**
     * @brief Translates a 2D point on the viewport into a 3D point at a specified distance
     * from the near view plane.
     *
     * @param[in] point           The 2D coordinates that represent the location on the GL canvas.
     * @param[in] viewDepth       The normalized depth by which to extend the 2D coordinate into
     *                            the scene. Zero is on the near view plane, and one is on the far
     *                            view plane.
     * @param[in] modelMatrix     The matrix of transformations applied to the model.
     *
     * @returns A 3D point representing the 2D canvas coordinates in 3D world coordinates.
     */
    QVector3D Unproject(const QPoint& point, float viewDepth, const QMatrix4x4& modelMatrix) const
        noexcept;

    /**
     * @brief Maps the 2D coordinates of the OpenGL Qt widget to the coordinates used by OpenGL.
     *
     * The difference between the two is that OpenGL has positive Y extending upwards, while Qt
     * has it extending downwards.
     *
     * @param[in] widgetCoordinates     2D Qt widget coordinates.
     *
     * @returns The converted point.
     */
    QPoint MapToOpenGLViewport(const QPoint& widgetCoordinates) const noexcept;

    /**
     * @brief Sets the size of the OpenGL canvas viewport.
     *
     * @param[in] bounds          The current size of the viewport.
     */
    void SetViewport(const QRect& prefixedSize) noexcept;

    /**
     * @brief Retrieves the dimensions of the viewport.
     *
     * These will be the same dimensions as the OpenGL canvas.
     *
     * @returns the size of the current viewport represented as a rectangle.
     */
    QRect GetViewport() const noexcept;

    /**
     * @brief Sets the field of view used in the perspective matrix.
     *
     * @param[in] angle           In degrees.
     */
    void SetFieldOfView(int angle) noexcept;

    /**
     * @brief Retrives the camera's current field of view.
     *
     * @returns The current field of view in degrees.
     */
    int GetVerticalFieldOfView() const noexcept;

    /**
     * @brief Increases the field of view by five degrees.
     */
    void IncreaseFieldOfView() noexcept;

    /**
     * @brief Decreases the field of view by five degrees.
     */
    void DecreaseFieldOfView() noexcept;

    /**
     * @brief Shoots a ray into the scene starting at the specified point on the
     * widget canvas.
     *
     * @param[in] widgetCoordinates   2D widget coordinates
     *
     * @returns A ray extending into the scene from the near plane to the far plane.
     */
    Ray ShootRayIntoScene(const QPoint& widgetCoordinates) const noexcept;

    /**
     * @brief Determines whether a point is in front of the camera plane.
     *
     * @param[in] point           The point being targeted.
     *
     * @returns True if the point in question lies in front of the camera's near view plane. Note
     * that the near plane still lies a little bit in front of camera's actual position.
     * Returns false if the point lies behind the camera's near view plane.
     */
    bool IsPointInFrontOfCamera(const QVector3D& point) const noexcept;

    /**
     * @brief Sets the near plane distance.
     *
     * @note Avoiding setting this too close, because that will kill your precision at distance,
     * which, in turn, will likely cause very obvious Z-fighting.
     *
     * @param[in] nearPlane         The new plane distance.
     */
    void SetNearPlane(float nearPlane) noexcept;

    /**
     * @brief Retrieves the distance from the camera's focal plane to the near viewing plane.
     *
     * @returns Distance to near plane.
     */
    float GetNearPlane() const noexcept;

    /**
     * @brief Sets the far plane distance.
     *
     * @param[in] farPlane        The new plane distance.
     */
    void SetFarPlane(float farPlane) noexcept;

    /**
     * @brief Retrieves the distance from the camera's focal plane to the far viewing plane.
     *
     * @returns Distance to far plane.
     */
    float GetFarPlane() const noexcept;

    /**
     * @returns The current aspect ratio.
     */
    float GetAspectRatio() const noexcept;

  private:
    QVector3D m_position;

    QRect m_viewport;

    double m_yaw{ 0.0 };
    double m_pitch{ 0.0 };

    int m_fieldOfView{ 45 };

    float m_aspectRatio{ 1.0f };
    float m_nearPlane{ 1.0f };
    float m_farPlane{ 2000.0f };
};

#endif // CAMERA_H
