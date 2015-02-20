#include "glCanvas.h"

#include "camera.h"

#include <QMouseEvent>
#include <QOpenGLShader>

#include <QTimer>
#include <QtMath>

#include <iostream>

/*
 * AWESOME RESOURCES:
 * ------------------
 * https://github.com/advancingu/Qt5OppenGL
 * www.tomdalling.com/
 * https://github.com/qtproject/learning-guides/tree/master/openGL_tutorial
 */

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
     m_lastFrameTimeStamp(std::chrono::system_clock::now()),
     m_vertexColorBuffer(QOpenGLBuffer::VertexBuffer),
     m_vertexPositionBuffer(QOpenGLBuffer::VertexBuffer)
{
   // Set up the camera:
   m_camera.SetAspectRatio(780.0f / 580.0f);
   m_camera.SetPosition(QVector3D(0, 0, m_distance));

   // Set keyboard and mouse focus:
   setFocusPolicy(Qt::StrongFocus);

   QSurfaceFormat format;
   format.setDepthBufferSize(24);
   setFormat(format);

   // Set the target frame rate:
   QTimer* timer = new QTimer(this);
   connect(timer, SIGNAL(timeout()), this, SLOT(update()));
   timer->start(10);
}

GLCanvas::~GLCanvas()
{
}

void GLCanvas::PrepareShaderProgram()
{
   if (!m_shaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/vertexShader.vert"))
   {
      std::cout << "Error adding vertex shader!" << std::endl;
   }

   if (!m_shaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/fragmentShader.frag"))
   {
      std::cout << "Error adding fragment shader!" << std::endl;
   }

   m_shaderProgram.link();
}

void GLCanvas::PrepareVertexBuffers()
{
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

   m_VAO.create();
   m_VAO.bind();

   m_vertexPositionBuffer.create();
   m_vertexPositionBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
   m_vertexPositionBuffer.bind();
   m_vertexPositionBuffer.allocate(m_vertices.constData(), m_vertices.size() * 3 * sizeof(GLfloat));

   m_vertexColorBuffer.create();
   m_vertexColorBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
   m_vertexColorBuffer.bind();
   m_vertexColorBuffer.allocate(m_colors.constData(), m_colors.size() * 3 * sizeof(GLfloat));

   m_shaderProgram.bind();

   m_shaderProgram.setUniformValue("mvpMatrix", m_camera.GetMatrix());

   m_vertexPositionBuffer.bind();
   m_shaderProgram.enableAttributeArray("vertex");
   m_shaderProgram.setAttributeBuffer("vertex", GL_FLOAT, /* offset = */ 0, /* tupleSize = */ 3);

   m_vertexColorBuffer.bind();
   m_shaderProgram.enableAttributeArray("color");
   m_shaderProgram.setAttributeBuffer("color", GL_FLOAT, /* offset = */ 0, /* tupleSize = */ 3);

   m_shaderProgram.release();
   m_vertexPositionBuffer.release();
   m_vertexColorBuffer.release();
   m_VAO.release();
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

   PrepareShaderProgram();
   PrepareVertexBuffers();
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
   const static float MOUSE_SENSITIVITY = 0.5f;

   const float deltaX = event->x() - m_lastMousePosition.x();
   const float deltaY = event->y() - m_lastMousePosition.y();

   if (event->buttons() & Qt::LeftButton)
   {
      m_camera.OffsetOrientation(MOUSE_SENSITIVITY * deltaY, MOUSE_SENSITIVITY * deltaX);
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
        m_camera.IncreaseFieldOfView();
      }
      else if (delta > 0)
      {
         m_camera.DecreaseFieldOfView();
      }
   }

   event->accept();
}

void GLCanvas::HandleCameraMovement()
{
   const static float MOVE_SPEED = 0.001f;

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
      m_camera.OffsetPosition(millisecondsElapsed.count() * MOVE_SPEED * m_camera.Forward());
   }

   if (isKeyADown)
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * MOVE_SPEED * m_camera.Left());
   }

   if (isKeySDown)
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * MOVE_SPEED * m_camera.Backward());
   }

   if (isKeyDDown)
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * MOVE_SPEED * m_camera.Right());
   }
}

void GLCanvas::paintGL()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   const auto currentTime = std::chrono::system_clock::now();
   const auto millisecondsElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now() - m_lastFrameTimeStamp).count();

   m_parent.setWindowTitle(QString::fromStdString("D-Viz @ ") +
      QString::number((int) (1000.0 / millisecondsElapsed)) + QString::fromStdString(" fps [*]"));

   HandleCameraMovement();

   m_shaderProgram.bind();
   m_shaderProgram.setUniformValue("mvpMatrix", m_camera.GetMatrix());

   m_VAO.bind();

   glDrawArrays(GL_TRIANGLES, /* first = */ 0, /* count = */ m_vertices.size());

   m_shaderProgram.release();
   m_VAO.release();

   m_lastFrameTimeStamp = currentTime;
}
