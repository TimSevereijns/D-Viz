#ifndef GLCANVAS_H
#define GLCANVAS_H

#include "camera.h"
#include "keyboardManager.h"

#include <chrono>

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QVector3D>

struct Light
{
   QVector3D position;
   QVector3D intensity;

   Light()
      : position(QVector3D(0, 0, 0)),
        intensity(QVector3D(1, 1, 1))
   {
   }

   Light(const QVector3D& lightPosition, const QVector3D& lightIntensity)
      : position(lightPosition),
        intensity(lightIntensity)
   {
   }

   void SetPosition(const QVector3D& newPosition)
   {
      position = newPosition;
   }
};

class GLCanvas : public QOpenGLWidget, protected QOpenGLFunctions
{
   Q_OBJECT

   public:
      explicit GLCanvas(QWidget* parent = nullptr);
      ~GLCanvas();

      QSize sizeHint() const override;

   protected:
      void initializeGL() override;
      void resizeGL(int width, int height) override;
      void paintGL() override;

      void keyPressEvent(QKeyEvent* event) override;
      void keyReleaseEvent(QKeyEvent* event) override;

      void mousePressEvent(QMouseEvent* event) override;
      void mouseMoveEvent(QMouseEvent* event) override;
      void wheelEvent(QWheelEvent* event) override;

   private:
      void HandleCameraMovement();

      void PrepareVisualizationShaderProgram();
      void PrepareOriginMarkerShaderProgram();

      void PrepareVisualizationVertexBuffers();
      void PrepareOriginMarkerVertexBuffers();

      QWidget& m_parent;

      Camera m_camera;

      Light m_light;

      KeyboardManager m_keyboardManager;

      QMatrix4x4 m_projectionMatrix;

      QOpenGLShaderProgram m_originMarkerShaderProgram;
      QOpenGLShaderProgram m_visualizationShaderProgram;

      QOpenGLVertexArrayObject m_visualizationVAO;
      QOpenGLVertexArrayObject m_originMarkerVAO;

      QOpenGLBuffer m_visualizationVertexPositionBuffer;
      QOpenGLBuffer m_visualizationVertexColorBuffer;

      QOpenGLBuffer m_originMarkerVertexPositionBuffer;
      QOpenGLBuffer m_originMarkerVertexColorBuffer;

      QVector<QVector3D> m_originMarkerVertices;
      QVector<QVector3D> m_originMarkerColors;

      QVector<QVector3D> m_visualizationVertices;
      QVector<QVector3D> m_visualizationColors;

      double m_distance;
      double m_movementSpeed;

      std::chrono::system_clock::time_point m_lastFrameTimeStamp;

      QPoint m_lastMousePosition;

   signals:

   public slots:
};

#endif // GLCANVAS_H
