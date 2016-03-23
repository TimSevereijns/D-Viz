#ifndef CAMERA_H
#define CAMERA_H

#include <QMatrix3x3>
#include <QMatrix4x4>
#include <QRect>

#include <Qt3DCore/QRay3D>

/**
 * @brief The Camera class represents the camera through which the scene is observed.
 */
class Camera
{
   public:
      /**
       * @brief Default constructor.
       */
      Camera();

      /**
       * @brief Retrieves the camera's current position within 3D space.
       *
       * @returns The position of the camera within the scene.
       */
      const QVector3D& GetPosition() const;

      /**
       * @brief Sets the position of the camera in the scene.
       *
       * @param[in] newPosition     The new location of the camera.
       */
      void SetPosition(const QVector3D& newPosition);

      /**
       * @brief Offsets the position of the camera by the specified amount relative to the current
       * position of the camera.
       *
       * @param[in] offset          The relative offset to apply.
       */
      void OffsetPosition(const QVector3D& offset);

      /**
       * @brief Retrieves the camera's current orientation matrix.
       *
       * @returns The current orientation of the camera; that is, pitch, roll, and yaw.
       */
      QMatrix4x4 GetOrientation() const;

      /**
       * @brief Sets the current camera orientation in terms of pitch and yaw.
       *
       * There is currently no roll support.
       *
       * @param[in] pitch           In degrees.
       * @param[in] yaw             In degrees.
       */
      void OffsetOrientation(float pitch, float yaw);

      /**
       * @brief Will orient the camera so that the specified point is within view.
       *
       * @param[in] point           The point to be brought into view.
       *
       * @todo Fix this function; it currently doesn't bring the point into view.
       */
      void LookAt(const QVector3D& point);

      /**
       * @brief Retrieves a vector pointing forward from the camera.
       *
       * @returns A vector that points forward relative to the current camera's position and
       * orientation.
       */
      QVector3D Forward() const;

      /**
       * @brief Retrieves a vector pointing backswards from the camera.
       *
       * @returns A vector that points backwards relative to the current camera's position and
       * orientation.
       */
      QVector3D Backward() const;

      /**
       * @brief Retrieves a vector pointing to the right of the camera.
       *
       * @returns A vector that points to the right relative to the current camera's position and
       * orientation.
       */
      QVector3D Right() const;

      /**
       * @brief Retrieves a vector pointing to the left of the camera.
       *
       * @returns A vector that points to the left relative to the current camera's position and
       * orientation.
       */
      QVector3D Left() const;

      /**
       * @brief Retrieves a vector pointing up from the camera.
       *
       * @returns A vector that points up relative to the current camera's position and orientation.
       */
      QVector3D Up() const;

      /**
       * @brief Retrieves a vector pointing down from the camera.
       *
       * @returns A vector that points down relative to the current camera's position and
       * orientation.
       */
      QVector3D Down() const;

      /**
       * @brief Retrieves the camera's current projection matrix.
       *
       * @returns The current projection matrix.
       */
      QMatrix4x4 GetProjectionMatrix() const;

      /**
       * @brief Retrieves the camera's current view matrix.
       *
       * @returns The current view matrix.
       */
      QMatrix4x4 GetViewMatrix() const;

      /**
       * @brief Retrieves the camera's projection-view matrix.
       *
       * @returns The projection matrix multiplied by the view matrix.
       */
      QMatrix4x4 GetProjectionViewMatrix() const;

      /**
       * @brief Translates a 2D point on the viewport into a 3D point at a specified distance
       * from the near view plane.
       *
       * @note Source: https://github.com/Cavewhere/cavewhere/blob/
       *               9121490139b3e0f046fc0086b86f0e1659d3bc8e/src/cwCamera.cpp
       *
       * @param[in] point           The 2D coordinates that represent the location on the GL canvas.
       * @param[in] viewDepth       The normalized depth by which to extend the 2D coordinate into
       *                            the scene. Zero is on the near view plane, and one is on the far
       *                            view plane.
       * @param[in] modelMatrix     The matrix of transformations applied to the model.
       *
       * @returns A 3D point representing the 2D canvas coordinates in 3D world coordinates.
       */
      QVector3D Unproject(const QPoint& point, float viewDepth, QMatrix4x4 modelMatrix) const;

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
      QPoint MapToOpenGLViewport(const QPoint& widgetCoordinates) const;

      /**
       * @brief Sets the current aspect ratio that factors into the perspective matrix.
       *
       * @param[in] ratio           The aspect ratio of the viewport.
       */
      void SetAspectRatio(const float ratio);

      /**
       * @brief Sets the size of the OpenGL canvas viewport.
       *
       * @param[in] bounds          The current size of the viewport.
       */
      void SetViewport(const QRect& size);

      /**
       * @brief Retrieves the dimensions of the viewport.
       *
       * These will be the same dimensions as the OpenGL canvas.
       *
       * @returns the size of the current viewport represented as a rectangle.
       */
      QRect GetViewport() const;

      /**
       * @brief Sets the field of view that will factor into the perspective matrix.
       *
       * @param[in] angle           In degrees.
       */
      void SetFieldOfView(const float angle);

      /**
       * @brief Retrives the camera's current field of view.
       *
       * @returns The current field of view in degrees.
       */
      float GetFieldOfView() const;

      /**
       * @brief Increases the field of view by five degrees.
       */
      void IncreaseFieldOfView();

      /**
       * @brief Decreases the field of view by five degrees.
       */
      void DecreaseFieldOfView();

      /**
       * @brief Shoots a ray into the scene starting at the specified point on the
       * widget canvas.
       *
       * @param[in] widgetCoordinates   2D widget coordinates
       *
       * @returns A ray extending into the scene from the near plane to the far plane.
       */
      Qt3D::QRay3D ShootRayIntoScene(const QPoint& widgetCoordinates) const;

      /**
       * @brief Determines whether a point is in front of the camera plane.
       *
       * @param[in] point           The point being targeted.
       *
       * @returns True if the point in question lies in front of the camera's positional plane. Note
       * that the near plane still lies a little bit in front of positional plane. Returns false if
       * the point lies behind the camera.
       */
      bool IsPointInFrontOfCamera(const QVector3D& point) const;

      float GetNearPlane() const;

      float GetFarPlane() const;

   private:
      QVector3D m_position;

      QRect m_viewport;

      double m_horizontalAngle;
      double m_verticalAngle;

      float m_fieldOfView;
      float m_aspectRatio;
      float m_nearPlane;
      float m_farPlane;
};

#endif // CAMERA_H
