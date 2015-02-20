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

class GLCanvas : public QOpenGLWidget, protected QOpenGLFunctions
{
   Q_OBJECT

   public:
      explicit GLCanvas(QWidget* parent = nullptr);
      ~GLCanvas();

      QSize sizeHint() const;

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
      void PrepareShaderProgram();
      void PrepareVertexBuffers();

      QWidget& m_parent;

      Camera m_camera;

      KeyboardManager m_keyboardManager;

      QMatrix4x4 m_projectionMatrix;

      QOpenGLShaderProgram m_shaderProgram;

      QOpenGLVertexArrayObject m_VAO;

      QOpenGLBuffer m_vertexPositionBuffer;
      QOpenGLBuffer m_vertexColorBuffer;

      QVector<QVector3D> m_vertices;
      QVector<QVector3D> m_colors;

      double m_distance;

      std::chrono::system_clock::time_point m_lastFrameTimeStamp;

      QPoint m_lastMousePosition;

   signals:

   public slots:
};

#endif // GLCANVAS_H
