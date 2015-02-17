#include "glCanvas.h"

#include "camera.h"

#include <QMouseEvent>
#include <QOpenGLShader>

#include <QTimer>
#include <QtMath>

#include <iostream>

namespace
{
   inline bool keyPressHelper(KeyboardManager& keyboardManager, QKeyEvent& event,
                              KeyboardManager::KEY_STATE state)
   {
      const Qt::Key pressedKey = static_cast<Qt::Key>(event.key());
      switch (pressedKey)
      {
         case Qt::Key_W: keyboardManager.UpdateKeyState(pressedKey, state); return true;
         case Qt::Key_A: keyboardManager.UpdateKeyState(pressedKey, state); return true;
         case Qt::Key_S: keyboardManager.UpdateKeyState(pressedKey, state); return true;
         case Qt::Key_D: keyboardManager.UpdateKeyState(pressedKey, state); return true;
      }

      return false;
   }
}

GLCanvas::GLCanvas(QWidget* parent)
   : QOpenGLWidget(parent),
     m_parent(*parent),
     m_distance(2.5),
     m_lastFrameTimeStamp(std::chrono::system_clock::now())
{
   m_camera.SetAspectRatio(780.0f / 580.0f);
   m_camera.SetPosition(QVector3D(0, 0, m_distance));

   setFocusPolicy(Qt::StrongFocus);

   QSurfaceFormat format;
   format.setDepthBufferSize(24);
   setFormat(format);

   QTimer* timer = new QTimer(this);
   connect(timer, SIGNAL(timeout()), this, SLOT(update()));
   timer->start(30);
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
   initializeOpenGLFunctions();

   glEnable(GL_DEPTH_TEST);
   //glEnable(GL_CULL_FACE);

   //glClearColor(QColor(Qt::gray));

   m_shader.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/vertexShader.vert");
   m_shader.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/fragmentShader.frag");
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

   glViewport(0, 0, width, height);
}

void GLCanvas::keyPressEvent(QKeyEvent* const event)
{
   if (event->isAutoRepeat())
   {
      event->ignore();
      return;
   }

   const auto keyState = KeyboardManager::KEY_STATE::DOWN;
   const bool wasKeyRecognized = keyPressHelper(m_keyboardManager, *event, keyState);
   if (wasKeyRecognized)
   {
      event->accept();
   }
   else
   {
      QOpenGLWidget::keyPressEvent(event);
   }
}

void GLCanvas::keyReleaseEvent(QKeyEvent* const event)
{
   if (event->isAutoRepeat())
   {
      event->ignore();
      return;
   }

   const auto keyState = KeyboardManager::KEY_STATE::UP;
   const bool wasKeyRecognized = keyPressHelper(m_keyboardManager, *event, keyState);
   if (wasKeyRecognized)
   {
      event->accept();
   }
   else
   {
      QOpenGLWidget::keyReleaseEvent(event);
   }
}

void GLCanvas::mousePressEvent(QMouseEvent* const event)
{
   m_lastMousePosition = event->pos();
   event->accept();
}

void GLCanvas::mouseMoveEvent(QMouseEvent* const event)
{
   //std::cout << "Mouse moving..." << std::endl;

   const static float mouseSensitivity = 0.5f;

   const float deltaX = event->x() - m_lastMousePosition.x();
   const float deltaY = event->y() - m_lastMousePosition.y();

   if (event->buttons() & Qt::LeftButton)
   {
      m_camera.OffsetOrientation(mouseSensitivity * deltaY, mouseSensitivity * deltaX);
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
   }

   event->accept();
}

void GLCanvas::HandleCameraMovement()
{
   const static double moveSpeed = 0.001;

   const auto millisecondsElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
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
      m_camera.OffsetPosition(millisecondsElapsed.count() * moveSpeed * m_camera.Forward());
   }

   if (isKeyADown)
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * moveSpeed * m_camera.Left());
   }

   if (isKeySDown)
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * moveSpeed * m_camera.Backward());
   }

   if (isKeyDDown)
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * moveSpeed * m_camera.Right());
   }
}

void GLCanvas::paintGL()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   const auto currentTime = std::chrono::system_clock::now();
   const auto millisecondsElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - m_lastFrameTimeStamp);

   QString windowTitle = QString::fromStdString("D-Viz ") +
      QString::number((int) (1000.0 / millisecondsElapsed.count())) +
      QString::fromStdString(" fps [*]");
   m_parent.setWindowTitle(windowTitle);

   HandleCameraMovement();

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

   m_lastFrameTimeStamp = currentTime;

//   if (hasFocus())
//   {
//      update();
//   }
//   else
//   {
//      m_parent.setWindowTitle("D-Viz - Drawing Suspended");
//   }
}
