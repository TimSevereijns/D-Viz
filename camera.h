#ifndef CAMERA_H
#define CAMERA_H

#include <QMatrix3x3>
#include <QMatrix4x4>
#include <QRect>

class Camera
{
   public:
      Camera();

      /**
       * @brief GetPosition
       *
       * @returns the position of the camera within the scene in which it resides.
       */
      const QVector3D& GetPosition() const;

      /**
       * @brief SetPosition sets the position of the camera in the scene.
       *
       * @param[in] newPosition     The new location of the camera.
       */
      void SetPosition(const QVector3D& newPosition);

      /**
       * @brief OffsetPosition offsets the position of the camera by the specified amount relative
       * to the current position of the camera.
       *
       * @param[in] offset          The relative offset to apply.
       */
      void OffsetPosition(const QVector3D& offset);

      /**
       * @brief GetOrientation
       * @returns the current orientation of the camera; that is, pitch, roll, and yaw.
       */
      QMatrix4x4 GetOrientation() const;

      /**
       * @brief OffsetOrientation sets the current camera orientation in terms of pitch and yaw.
       * There is currently no roll support.
       *
       * @param[in] pitch           In degrees.
       * @param[in] yaw             In degrees.
       */
      void OffsetOrientation(float pitch, float yaw);

      /**
       * @brief LookAt
       * @param position
       */
      void LookAt(const QVector3D& position);

      /**
       * @brief Forward
       *
       * @returns a vector that points forward relative to the current camera's position and
       * orientation.
       */
      QVector3D Forward() const;

      /**
       * @brief Backward
       *
       * @returns a vector that points backwards relative to the current camera's position and
       * orientation.
       */
      QVector3D Backward() const;

      /**
       * @brief Right
       *
       * @returns a vector that points to the right relative to the current camera's position and
       * orientation.
       */
      QVector3D Right() const;

      /**
       * @brief Left
       *
       * @returns a vector that points to the left relative to the current camera's position and
       * orientation.
       */
      QVector3D Left() const;

      /**
       * @brief Up
       *
       * @returns a vector that points up relative to the current camera's position and orientation.
       */
      QVector3D Up() const;

      /**
       * @brief Down
       *
       * @returns a vector that points down relative to the current camera's position and
       * orientation.
       */
      QVector3D Down() const;

      /**
       * @brief GetProjectionMatrix
       *
       * @returns the current projection matrix.
       */
      QMatrix4x4 GetProjectionMatrix() const;

      /**
       * @brief GetViewMatrix
       *
       * @returns the current view matrix.
       */
      QMatrix4x4 GetViewMatrix() const;

      /**
       * @brief GetProjectionViewMatrix
       *
       * @returns the projection matrix multiplied by the view matrix.
       */
      QMatrix4x4 GetProjectionViewMatrix() const;

      /**
       * @brief Unproject
       * @note Source: https://github.com/Cavewhere/cavewhere/blob/
       *               9121490139b3e0f046fc0086b86f0e1659d3bc8e/src/cwCamera.cpp
       *
       * @param point
       * @param viewDepth
       * @param modelMatrix
       *
       * @returns
       */
      QVector3D Unproject(QPoint point, float viewDepth, QMatrix4x4 modelMatrix) const;

      /**
       * @brief MapToOpenGLViewport maps the 2D coordinates of the OpenGL Qt widget to the
       * coordinates used by OpenGL. The difference between the two is that one has the positive Y
       * going up, while the other has them going down.
       *
       * @param[in] coordinatesOnQtWidget
       *
       * @returns the converted point.
       */
      QPoint MapToOpenGLViewport(const QPoint& coordinatesOnQtWidget) const;

      /**
       * @brief SetAspectRatio sets the current aspect ratio that factors into the perspective
       * matrix.
       *
       * @param[in] ratio           The aspect ratio of the viewport.
       */
      void SetAspectRatio(const float ratio);

      /**
       * @brief SetViewport sets the size of the OpenGL canvas viewport.
       *
       * @param[in] bounds          The current size of the viewport.
       */
      void SetViewport(const QRect& size);

      /**
       * @brief GetViewport
       *
       * @returns the size of the current viewport.
       */
      QRect GetViewport() const;

      /**
       * @brief SetFieldOfView sets the field of view that will factor into the perspective matrix.
       *
       * @param[in] angle           In degrees.
       */
      void SetFieldOfView(const float angle);

      /**
       * @brief GetFieldOfView
       *
       * @returns the current field of view in degrees.
       */
      float GetFieldOfView() const;

      /**
       * @brief IncreaseFieldOfView increases the field of view by five degrees.
       */
      void IncreaseFieldOfView();

      /**
       * @brief DecreaseFieldOfView decreases the field of view by five degrees.
       */
      void DecreaseFieldOfView();

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
