#ifndef CAMERA_H
#define CAMERA_H

#include <QMatrix3x3>
#include <QMatrix4x4>
#include <QRect>

class Camera
{
   public:
      Camera();

      const QVector3D& GetPosition() const;

      void SetPosition(const QVector3D& newPosition);
      void OffsetPosition(const QVector3D& offset);

      QMatrix4x4 GetOrientation() const;
      void OffsetOrientation(float verticalAngle, float horizontalAngle);

      void LookAt(const QVector3D& position);

      QVector3D Forward() const;
      QVector3D Backward() const;
      QVector3D Right() const;
      QVector3D Left() const;
      QVector3D Up() const;
      QVector3D Down() const;

      QMatrix4x4 GetProjectionMatrix() const;
      QMatrix4x4 GetViewMatrix() const;
      QMatrix4x4 GetMatrix() const;

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

      QPoint MapToOpenGLViewport(const QPoint& coordinatesOnQtWidget) const;

      void SetAspectRatio(const float ratio);

      void SetViewport(const QRect& bounds);
      QRect GetViewport() const;

      void SetFieldOfView(const float angle);
      float GetFieldOfView() const;

      void IncreaseFieldOfView();
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
