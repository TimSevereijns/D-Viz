#include "glCanvas.h"

#include "camera.h"
#include "Visualizations/sliceAndDiceTreemap.h"
#include "Visualizations/squarifiedTreemap.h"

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
 * https://github.com/g-truc/ogl-samples/tree/master/tests
 */

namespace
{
   /**
    * @brief keyPressHelper is a helper function to update the press state of a keyboard key.
    * @param keyboardManager        The keyboard manager on which to update the key state.
    * @param key                    The QKeyEvent that triggered the call.
    * @param state                  Whether the key in question is up (released) or down (pressed).
    * @returns true if the key exists and was updated in the keyboard manager.
    */
   inline bool keyPressHelper(KeyboardManager& keyboardManager, Qt::Key key,
                              KeyboardManager::KEY_STATE state)
   {
      switch (key)
      {
         case Qt::Key_W: keyboardManager.UpdateKeyState(key, state); return true;
         case Qt::Key_A: keyboardManager.UpdateKeyState(key, state); return true;
         case Qt::Key_S: keyboardManager.UpdateKeyState(key, state); return true;
         case Qt::Key_D: keyboardManager.UpdateKeyState(key, state); return true;
      }

      return false;
   }

   /**
    * @brief CreateOriginMarkerVertices returns the vertices needed to render the coordinate
    *        system origin marker.
    * @returns a vector of vertices.
    */
   QVector<QVector3D> CreateOriginMarkerVertices()
   {
      const float markerAxisLength = 3;

      QVector<QVector3D> marker;
      marker
         << QVector3D(0.0f, 0.0f, 0.0f) << QVector3D(markerAxisLength, 0.0f, 0.0f)   // X-axis
         << QVector3D(0.0f, 0.0f, 0.0f) << QVector3D(0.0f, markerAxisLength, 0.0f)   // Y-axis
         << QVector3D(0.0f, 0.0f, 0.0f) << QVector3D(0.0f, 0.0f, -markerAxisLength)  // Z-axis

         // Grid (Z-axis):
         << QVector3D( 0.0f, 0.0f, -3.0f) << QVector3D( 0.0f, 0.0f, -10.0f)
         << QVector3D( 1.0f, 0.0f,  0.0f) << QVector3D( 1.0f, 0.0f, -10.0f)
         << QVector3D( 2.0f, 0.0f,  0.0f) << QVector3D( 2.0f, 0.0f, -10.0f)
         << QVector3D( 3.0f, 0.0f,  0.0f) << QVector3D( 3.0f, 0.0f, -10.0f)
         << QVector3D( 4.0f, 0.0f,  0.0f) << QVector3D( 4.0f, 0.0f, -10.0f)
         << QVector3D( 5.0f, 0.0f,  0.0f) << QVector3D( 5.0f, 0.0f, -10.0f)
         << QVector3D( 6.0f, 0.0f,  0.0f) << QVector3D( 6.0f, 0.0f, -10.0f)
         << QVector3D( 7.0f, 0.0f,  0.0f) << QVector3D( 7.0f, 0.0f, -10.0f)
         << QVector3D( 8.0f, 0.0f,  0.0f) << QVector3D( 8.0f, 0.0f, -10.0f)
         << QVector3D( 9.0f, 0.0f,  0.0f) << QVector3D( 9.0f, 0.0f, -10.0f)
         << QVector3D(10.0f, 0.0f,  0.0f) << QVector3D(10.0f, 0.0f, -10.0f)

         // Grid (X-axis):
         << QVector3D(3.0f, 0.0f,   0.0f) << QVector3D(10.0f, 0.0f,   0.0f)
         << QVector3D(0.0f, 0.0f,  -1.0f) << QVector3D(10.0f, 0.0f,  -1.0f)
         << QVector3D(0.0f, 0.0f,  -2.0f) << QVector3D(10.0f, 0.0f,  -2.0f)
         << QVector3D(0.0f, 0.0f,  -3.0f) << QVector3D(10.0f, 0.0f,  -3.0f)
         << QVector3D(0.0f, 0.0f,  -4.0f) << QVector3D(10.0f, 0.0f,  -4.0f)
         << QVector3D(0.0f, 0.0f,  -5.0f) << QVector3D(10.0f, 0.0f,  -5.0f)
         << QVector3D(0.0f, 0.0f,  -6.0f) << QVector3D(10.0f, 0.0f,  -6.0f)
         << QVector3D(0.0f, 0.0f,  -7.0f) << QVector3D(10.0f, 0.0f,  -7.0f)
         << QVector3D(0.0f, 0.0f,  -8.0f) << QVector3D(10.0f, 0.0f,  -8.0f)
         << QVector3D(0.0f, 0.0f,  -9.0f) << QVector3D(10.0f, 0.0f,  -9.0f)
         << QVector3D(0.0f, 0.0f, -10.0f) << QVector3D(10.0f, 0.0f, -10.0f);

      return marker;
   }

   /**
    * @brief CreateOriginMarkerColors returns the vertex colors needed to paint the origin marker.
    * @returns a vector of vertex colors.
    */
   QVector<QVector3D> CreateOriginMarkerColors()
   {
      QVector<QVector3D> markerColors;
      markerColors
         << QVector3D(1.0f, 1.0f, 1.0f) << QVector3D(1.0f, 0.0f, 0.0f)  // X-axis (red)
         << QVector3D(1.0f, 1.0f, 1.0f) << QVector3D(0.0f, 1.0f, 0.0f)  // Y-axis (green)
         << QVector3D(1.0f, 1.0f, 1.0f) << QVector3D(0.0f, 0.0f, 1.0f)  // Z-axis (blue)

         // Grid (Z-axis):
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)

         // Grid (X-axis):
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f);

      return markerColors;
   }
}

GLCanvas::GLCanvas(QWidget* parent)
   : QOpenGLWidget(parent),
     m_parent(*parent),
     m_distance(2.5),
     m_movementSpeed(0.002f),
     m_lastFrameTimeStamp(std::chrono::system_clock::now()),
     m_visualizationVertexColorBuffer(QOpenGLBuffer::VertexBuffer),
     m_visualizationVertexPositionBuffer(QOpenGLBuffer::VertexBuffer)
{
   // Set up the camera:
   m_camera.SetAspectRatio(1200.0f / 800.0f);
   m_camera.SetPosition(QVector3D(0, 0, m_distance));

   // Set keyboard and mouse focus:
   setFocusPolicy(Qt::StrongFocus);

   QSurfaceFormat format;
   format.setDepthBufferSize(24);
   setFormat(format);

   // Set the target frame rate:
   QTimer* timer = new QTimer(this);
   connect(timer, SIGNAL(timeout()), this, SLOT(update()));
   timer->start(20);
}

GLCanvas::~GLCanvas()
{
}

void GLCanvas::PrepareOriginMarkerShaderProgram()
{
   if (!m_originMarkerShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,
      ":/Shaders/originMarkerVertexShader.vert"))
   {
      std::cout << "Error loading origin marker vertex shader!" << std::endl;
   }

   if (!m_originMarkerShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,
      ":/Shaders/originMarkerFragmentShader.frag"))
   {
      std::cout << "Error loading origin marker fragment shader!" << std::endl;
   }

   m_originMarkerShaderProgram.link();
}

void GLCanvas::PrepareVisualizationShaderProgram()
{
   if (!m_visualizationShaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,
      ":/Shaders/visualizationVertexShader.vert"))
   {
      std::cout << "Error loading visualization vertex shader!" << std::endl;
   }

   if (!m_visualizationShaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,
      ":/Shaders/visualizationFragmentShader.frag"))
   {
      std::cout << "Error loading visualization fragment shader!" << std::endl;
   }

   m_visualizationShaderProgram.link();
}

void GLCanvas::PrepareOriginMarkerVertexBuffers()
{
   m_originMarkerVertices << CreateOriginMarkerVertices();
   m_originMarkerColors << CreateOriginMarkerColors();

   m_originMarkerVAO.create();
   m_originMarkerVAO.bind();

   m_originMarkerVertexPositionBuffer.create();
   m_originMarkerVertexPositionBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
   m_originMarkerVertexPositionBuffer.bind();
   m_originMarkerVertexPositionBuffer.allocate(m_originMarkerVertices.constData(),
      m_originMarkerVertices.size() * 3 * sizeof(GLfloat));

   m_originMarkerVertexColorBuffer.create();
   m_originMarkerVertexColorBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
   m_originMarkerVertexColorBuffer.bind();
   m_originMarkerVertexColorBuffer.allocate(m_originMarkerColors.constData(),
      m_originMarkerColors.size() * 3 * sizeof(GLfloat));

   m_originMarkerShaderProgram.bind();

   m_originMarkerShaderProgram.setUniformValue("mvpMatrix", m_camera.GetMatrix());

   m_originMarkerVertexPositionBuffer.bind();
   m_originMarkerShaderProgram.enableAttributeArray("vertex");
   m_originMarkerShaderProgram.setAttributeBuffer("vertex", GL_FLOAT, /* offset = */ 0,
      /* tupleSize = */ 3);

   m_originMarkerVertexColorBuffer.bind();
   m_originMarkerShaderProgram.enableAttributeArray("color");
   m_originMarkerShaderProgram.setAttributeBuffer("color", GL_FLOAT, /* offset = */ 0,
      /* tupleSize = */ 3);

   m_originMarkerShaderProgram.release();
   m_originMarkerVertexPositionBuffer.release();
   m_originMarkerVertexColorBuffer.release();
   m_originMarkerVAO.release();
}

void GLCanvas::PrepareVisualizationVertexBuffers()
{
   //SliceAndDiceTreeMap treeMap{L"C:\\Users\\Tim"};
   //SquarifiedTreeMap treeMap{L"C:\\Users\\tsevereijns\\Pictures\\OK DST"};
   //SquarifiedTreeMap treeMap{L"C:\\Users\\Tim\\Documents\\GitHub\\D-Viz\\UnitTests"};
   SquarifiedTreeMap treeMap{L"C:\\Users\\Tim\\Documents\\Adobe\\Premiere Pro\\4.0\\Adobe Premiere Pro Preview Files\\Sendero.PRV"};
   treeMap.ScanDirectory();
   treeMap.ParseScan();

   m_visualizationVertices = treeMap.GetVertices();
   m_visualizationColors = treeMap.GetColors();

   m_visualizationVAO.create();
   m_visualizationVAO.bind();

   m_visualizationVertexPositionBuffer.create();
   m_visualizationVertexPositionBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
   m_visualizationVertexPositionBuffer.bind();
   m_visualizationVertexPositionBuffer.allocate(m_visualizationVertices.constData(),
      m_visualizationVertices.size() * 3 * sizeof(GLfloat));

   m_visualizationVertexColorBuffer.create();
   m_visualizationVertexColorBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
   m_visualizationVertexColorBuffer.bind();
   m_visualizationVertexColorBuffer.allocate(m_visualizationColors.constData(),
      m_visualizationColors.size() * 3 * sizeof(GLfloat));

   m_visualizationShaderProgram.bind();

   m_visualizationShaderProgram.setUniformValue("mvpMatrix", m_camera.GetMatrix());

   m_visualizationVertexPositionBuffer.bind();
   m_visualizationShaderProgram.enableAttributeArray("vertex");
   m_visualizationShaderProgram.setAttributeBuffer("vertex", GL_FLOAT, /* offset = */ 0,
      /* tupleSize = */ 3, /* stride = */ 6 * sizeof(GLfloat));

   m_visualizationVertexPositionBuffer.bind();
   m_visualizationShaderProgram.enableAttributeArray("normal");
   m_visualizationShaderProgram.setAttributeBuffer("normal", GL_FLOAT,
      /* offset = */ 3 * sizeof(GLfloat), /* tupleSize = */ 3, /* stride = */ 6 * sizeof(GLfloat));

   m_visualizationVertexColorBuffer.bind();
   m_visualizationShaderProgram.enableAttributeArray("color");
   m_visualizationShaderProgram.setAttributeBuffer("color", GL_FLOAT, /* offset = */ 0,
      /* tupleSize = */ 3);

   m_visualizationShaderProgram.release();
   m_visualizationVertexPositionBuffer.release();
   m_visualizationVertexColorBuffer.release();
   m_visualizationVAO.release();
}

QSize GLCanvas::sizeHint() const
{
   return QSize(m_parent.size().width(), m_parent.size().height());
}

void GLCanvas::initializeGL()
{
   initializeOpenGLFunctions();

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);

   PrepareVisualizationShaderProgram();
   PrepareOriginMarkerShaderProgram();

   PrepareVisualizationVertexBuffers();
   PrepareOriginMarkerVertexBuffers();

   m_light = Light(QVector3D(2, 2, 0), QVector3D(1, 1, 1));
}

void GLCanvas::resizeGL(int width, int height)
{
   // Avoid a divide-by-zero situation:
   if (height == 0)
   {
      height = 1;
   }

   glViewport(0, 0, width, height);
   m_camera.SetAspectRatio(static_cast<float>(width) / static_cast<float>(height));
}

void GLCanvas::keyPressEvent(QKeyEvent* const event)
{
   if (event->isAutoRepeat())
   {
      event->ignore();
      return;
   }

   if (event->key() == Qt::Key_Up)
   {
      m_movementSpeed *= 1.25f;
      std::cout << "Increasing move speed by 25%, now: " << m_movementSpeed << std::endl;
   }
   else if (event->key() == Qt::Key_Down)
   {
      m_movementSpeed *= 0.75f;
      std::cout << "Decreasing move speed by 25%, now: " << m_movementSpeed << std::endl;
   }

   const auto keyState = KeyboardManager::KEY_STATE::DOWN;
   const bool wasKeyRecognized = keyPressHelper(m_keyboardManager,
      static_cast<Qt::Key>(event->key()), keyState);
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
   const bool wasKeyRecognized = keyPressHelper(m_keyboardManager,
      static_cast<Qt::Key>(event->key()), keyState);
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
   //const static float MOVE_SPEED = 0.002f;

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
      m_camera.OffsetPosition(millisecondsElapsed.count() * m_movementSpeed * m_camera.Forward());
   }

   if (isKeyADown)
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * m_movementSpeed * m_camera.Left());
   }

   if (isKeySDown)
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * m_movementSpeed * m_camera.Backward());
   }

   if (isKeyDDown)
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * m_movementSpeed * m_camera.Right());
   }

   m_light.SetPosition(m_camera.GetPosition());
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

   // Draw origin marker:
   m_originMarkerShaderProgram.bind();
   m_originMarkerShaderProgram.setUniformValue("mvpMatrix", m_camera.GetMatrix());

   m_originMarkerVAO.bind();

   glDrawArrays(GL_LINES, /* first = */ 0, /* count = */ m_originMarkerVertices.size());

   m_originMarkerShaderProgram.release();
   m_originMarkerVAO.release();

   // Draw visualization:
   m_visualizationShaderProgram.bind();
   m_visualizationShaderProgram.setUniformValue("mvpMatrix", m_camera.GetMatrix());
   m_visualizationShaderProgram.setUniformValue("light.position", m_light.position);
   m_visualizationShaderProgram.setUniformValue("light.intensity", m_light.intensity);

   m_visualizationVAO.bind();

   glDrawArrays(GL_TRIANGLES, /* first = */ 0, /* count = */ m_visualizationVertices.size());

   m_visualizationShaderProgram.release();
   m_visualizationVAO.release();

   m_lastFrameTimeStamp = currentTime;
}
