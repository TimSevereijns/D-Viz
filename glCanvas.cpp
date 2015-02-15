#include "glCanvas.h"

#include "camera.h"

#include <QGLWidget>
#include <QMouseEvent>
#include <QtMath>

GLCanvas::GLCanvas(QWidget *parent)
   : QGLWidget(parent),
     m_alpha(25),
     m_beta(-25),
     m_distance(2.5),
     m_lastFrameTimeStamp(std::chrono::system_clock::now())
{
   m_camera.SetAspectRatio(780.0f / 580.0f);
   m_camera.SetPosition(QVector3D(0, 0, m_distance));
}

GLCanvas::~GLCanvas()
{
}

QSize GLCanvas::sizeHint() const
{
   return QSize(780, 580);
}

void GLCanvas::initializeGL()
{
   glEnable(GL_DEPTH_TEST);
   //glEnable(GL_CULL_FACE);

   qglClearColor(QColor(Qt::gray));

   m_shader.addShaderFromSourceFile(QGLShader::Vertex, ":/Shaders/vertexShader.vert");
   m_shader.addShaderFromSourceFile(QGLShader::Fragment, ":/Shaders/fragmentShader.frag");
   m_shader.link();

   m_vertices
     << QVector3D(-0.5, -0.5,  0.5) << QVector3D( 0.5, -0.5,  0.5) << QVector3D( 0.5,  0.5,  0.5) // Front
     << QVector3D( 0.5,  0.5,  0.5) << QVector3D(-0.5,  0.5,  0.5) << QVector3D(-0.5, -0.5,  0.5)
     << QVector3D( 0.5, -0.5, -0.5) << QVector3D(-0.5, -0.5, -0.5) << QVector3D(-0.5,  0.5, -0.5) // Back
     << QVector3D(-0.5,  0.5, -0.5) << QVector3D( 0.5,  0.5, -0.5) << QVector3D( 0.5, -0.5, -0.5)
     << QVector3D(-0.5, -0.5, -0.5) << QVector3D(-0.5, -0.5,  0.5) << QVector3D(-0.5,  0.5,  0.5) // Left
     << QVector3D(-0.5,  0.5,  0.5) << QVector3D(-0.5,  0.5, -0.5) << QVector3D(-0.5, -0.5, -0.5)
     << QVector3D( 0.5, -0.5,  0.5) << QVector3D( 0.5, -0.5, -0.5) << QVector3D( 0.5,  0.5, -0.5) // Right
     << QVector3D( 0.5,  0.5, -0.5) << QVector3D( 0.5,  0.5,  0.5) << QVector3D( 0.5, -0.5,  0.5)
     << QVector3D(-0.5,  0.5,  0.5) << QVector3D( 0.5,  0.5,  0.5) << QVector3D( 0.5,  0.5, -0.5) // Top
     << QVector3D( 0.5,  0.5, -0.5) << QVector3D(-0.5,  0.5, -0.5) << QVector3D(-0.5,  0.5,  0.5)
     << QVector3D(-0.5, -0.5, -0.5) << QVector3D( 0.5, -0.5, -0.5) << QVector3D( 0.5, -0.5,  0.5) // Bottom
     << QVector3D( 0.5, -0.5,  0.5) << QVector3D(-0.5, -0.5,  0.5) << QVector3D(-0.5, -0.5, -0.5);

   m_colors
      << QVector3D(1, 0, 0) << QVector3D(1, 0, 0) << QVector3D(1, 0, 0) // Front
      << QVector3D(1, 0, 0) << QVector3D(1, 0, 0) << QVector3D(1, 0, 0)
      << QVector3D(1, 0, 0) << QVector3D(1, 0, 0) << QVector3D(1, 0, 0) // Back
      << QVector3D(1, 0, 0) << QVector3D(1, 0, 0) << QVector3D(1, 0, 0)
      << QVector3D(0, 1, 0) << QVector3D(0, 1, 0) << QVector3D(0, 1, 0) // Left
      << QVector3D(0, 1, 0) << QVector3D(0, 1, 0) << QVector3D(0, 1, 0)
      << QVector3D(0, 1, 0) << QVector3D(0, 1, 0) << QVector3D(0, 1, 0) // Right
      << QVector3D(0, 1, 0) << QVector3D(0, 1, 0) << QVector3D(0, 1, 0)
      << QVector3D(0, 0, 1) << QVector3D(0, 0, 1) << QVector3D(0, 0, 1) // Top
      << QVector3D(0, 0, 1) << QVector3D(0, 0, 1) << QVector3D(0, 0, 1)
      << QVector3D(0, 0, 1) << QVector3D(0, 0, 1) << QVector3D(0, 0, 1) // Bottom
      << QVector3D(0, 0, 1) << QVector3D(0, 0, 1) << QVector3D(0, 0, 1);
}

void GLCanvas::resizeGL(int width, int height)
{
   // Avoid a divide-by-zero situation:
   if (height == 0)
   {
      height = 1;
   }

   m_projectionMatrix.setToIdentity();
   m_projectionMatrix.perspective(60.0f, (float) width / (float) height, 0.001f, 1000.0f);

   glViewport(0, 0, width, height);
}

void GLCanvas::keyPressEvent(QKeyEvent* const event)
{
   const Qt::Key pressedKey = static_cast<Qt::Key>(event->key());
   switch (pressedKey)
   {
      case Qt::Key_W: m_keyboardManager.UpdateKeyState(pressedKey, *event); break;
      case Qt::Key_A: m_keyboardManager.UpdateKeyState(pressedKey, *event); break;
      case Qt::Key_S: m_keyboardManager.UpdateKeyState(pressedKey, *event); break;
      case Qt::Key_D: m_keyboardManager.UpdateKeyState(pressedKey, *event); break;
   }
}

void GLCanvas::mousePressEvent(QMouseEvent* const event)
{
   m_lastMousePosition = event->pos();

   event->accept();
}

void GLCanvas::mouseMoveEvent(QMouseEvent* const event)
{
   const int deltaX = event->x() - m_lastMousePosition.x();
   const int deltaY = event->y() - m_lastMousePosition.y();

   if (event->buttons() & Qt::LeftButton) {
      m_alpha -= deltaX;
      while (m_alpha < 0)
      {
         m_alpha += 360;
      }
      while (m_alpha >= 360)
      {
         m_alpha -= 360;
      }

      m_beta -= deltaY;
      if (m_beta < -90)
      {
         m_beta = -90;
      }
      if (m_beta > 90)
      {
         m_beta = 90;
      }

      updateGL();
   }

   m_lastMousePosition = event->pos();

   event->accept();
}

void GLCanvas::wheelEvent(QWheelEvent* const event)
{
   const int delta = event->delta();

   if (event->orientation() == Qt::Vertical)
   {
      if (delta < 0)
      {
         m_distance *= 1.1;
      }
      else if (delta > 0)
      {
         m_distance *= 0.9;
      }

      updateGL();
   }

   event->accept();
}

void GLCanvas::HandleCameraMovement()
{
   const static double moveSpeed = 0.01;

   const std::chrono::duration<double> secondsElapsed =
      std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::system_clock::now() - m_lastFrameTimeStamp);

   const bool isKeyWDown = m_keyboardManager.IsKeyDown(Qt::Key_W);
   const bool isKeyADown = m_keyboardManager.IsKeyDown(Qt::Key_A);
   const bool isKeySDown = m_keyboardManager.IsKeyDown(Qt::Key_S);
   const bool isKeyDDown = m_keyboardManager.IsKeyDown(Qt::Key_D);

   if ((isKeyWDown && isKeySDown) || (isKeyADown && isKeyDDown))
   {
      return;
   }

   if (isKeyWDown)
   {
      m_camera.OffsetPosition(secondsElapsed.count() * moveSpeed * m_camera.Forward());
   }
   else if (isKeyADown)
   {
      m_camera.OffsetPosition(secondsElapsed.count() * moveSpeed * m_camera.Left());
   }
   else if (isKeySDown)
   {
      m_camera.OffsetPosition(secondsElapsed.count() * moveSpeed * m_camera.Backward());
   }
   else if (isKeyDDown)
   {
      m_camera.OffsetPosition(secondsElapsed.count() * moveSpeed * m_camera.Right());
   }
}

void GLCanvas::paintGL()
{
   m_lastFrameTimeStamp = std::chrono::system_clock::now();
   HandleCameraMovement();

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   QMatrix4x4 modelMatrix;
   QMatrix4x4 viewMatrix;

   QMatrix4x4 cameraTransformation;
   cameraTransformation.rotate(m_alpha, 0, 1, 0);
   cameraTransformation.rotate(m_beta, 1, 0, 0);

   //const QVector3D cameraPosition = cameraTransformation * QVector3D(0, 0, m_distance);
   //const QVector3D cameraUpDirection = cameraTransformation * QVector3D(0, 1, 0);

   m_camera.LookAt(QVector3D(0, 0, 0));

   //viewMatrix.lookAt(cameraPosition, QVector3D(0, 0, 0), cameraUpDirection);

   m_shader.bind();
   m_shader.setUniformValue("mvpMatrix", m_camera.GetMatrix());

   m_shader.setAttributeArray("vertex", m_vertices.constData());
   m_shader.enableAttributeArray("vertex");

   m_shader.setAttributeArray("color", m_colors.constData());
   m_shader.enableAttributeArray("color");

   glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());

   m_shader.disableAttributeArray("vertex");
   m_shader.disableAttributeArray("color");

   m_shader.release();
}
