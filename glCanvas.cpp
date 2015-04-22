#include "glCanvas.h"

#include "camera.h"
#include "mainwindow.h"
#include "Visualizations/sliceAndDiceTreemap.h"
#include "Visualizations/squarifiedTreemap.h"

#include <QMouseEvent>
#include <QOpenGLShader>

#include <QStatusBar>
#include <QTimer>

#include <iostream>
#include <sstream>
#include <stdexcept>

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
    *
    * @param keyboardManager        The keyboard manager on which to update the key state.
    * @param key                    The QKeyEvent that triggered the call.
    * @param state                  Whether the key in question is up (released) or down (pressed).
    *
    * @returns true if the key exists and was updated in the keyboard manager.
    */
   inline bool keyPressHelper(KeyboardManager& keyboardManager, const Qt::Key key,
      const KeyboardManager::KEY_STATE state)
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
    * system origin marker.
    *
    * @returns a vector of vertices.
    */
   QVector<QVector3D> CreateOriginMarkerVertices()
   {
      const float markerAxisLength = Visualization::ROOT_BLOCK_WIDTH;

      QVector<QVector3D> marker;
      marker
         << QVector3D(0.0f, 0.0f, 0.0f) << QVector3D(markerAxisLength, 0.0f, 0.0f)   // X-axis
         << QVector3D(0.0f, 0.0f, 0.0f) << QVector3D(0.0f, 100.0f, 0.0f)             // Y-axis
         << QVector3D(0.0f, 0.0f, 0.0f) << QVector3D(0.0f, 0.0f, -markerAxisLength)  // Z-axis

         // Grid (Z-axis):
         << QVector3D( 100.0f, 0.0f,  0.0f) << QVector3D( 100.0f, 0.0f, -1000.0f)
         << QVector3D( 200.0f, 0.0f,  0.0f) << QVector3D( 200.0f, 0.0f, -1000.0f)
         << QVector3D( 300.0f, 0.0f,  0.0f) << QVector3D( 300.0f, 0.0f, -1000.0f)
         << QVector3D( 400.0f, 0.0f,  0.0f) << QVector3D( 400.0f, 0.0f, -1000.0f)
         << QVector3D( 500.0f, 0.0f,  0.0f) << QVector3D( 500.0f, 0.0f, -1000.0f)
         << QVector3D( 600.0f, 0.0f,  0.0f) << QVector3D( 600.0f, 0.0f, -1000.0f)
         << QVector3D( 700.0f, 0.0f,  0.0f) << QVector3D( 700.0f, 0.0f, -1000.0f)
         << QVector3D( 800.0f, 0.0f,  0.0f) << QVector3D( 800.0f, 0.0f, -1000.0f)
         << QVector3D( 900.0f, 0.0f,  0.0f) << QVector3D( 900.0f, 0.0f, -1000.0f)
         << QVector3D(1000.0f, 0.0f,  0.0f) << QVector3D(1000.0f, 0.0f, -1000.0f)

         // Grid (X-axis):
         << QVector3D(0.0f, 0.0f,  -100.0f) << QVector3D(1000.0f, 0.0f,  -100.0f)
         << QVector3D(0.0f, 0.0f,  -200.0f) << QVector3D(1000.0f, 0.0f,  -200.0f)
         << QVector3D(0.0f, 0.0f,  -300.0f) << QVector3D(1000.0f, 0.0f,  -300.0f)
         << QVector3D(0.0f, 0.0f,  -400.0f) << QVector3D(1000.0f, 0.0f,  -400.0f)
         << QVector3D(0.0f, 0.0f,  -500.0f) << QVector3D(1000.0f, 0.0f,  -500.0f)
         << QVector3D(0.0f, 0.0f,  -600.0f) << QVector3D(1000.0f, 0.0f,  -600.0f)
         << QVector3D(0.0f, 0.0f,  -700.0f) << QVector3D(1000.0f, 0.0f,  -700.0f)
         << QVector3D(0.0f, 0.0f,  -800.0f) << QVector3D(1000.0f, 0.0f,  -800.0f)
         << QVector3D(0.0f, 0.0f,  -900.0f) << QVector3D(1000.0f, 0.0f,  -900.0f)
         << QVector3D(0.0f, 0.0f, -1000.0f) << QVector3D(1000.0f, 0.0f, -1000.0f);

      return marker;
   }

   /**
    * @brief CreateOriginMarkerColors returns the vertex colors needed to paint the origin marker.
    *
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

   /**
    * @brief SetStatusBarMessage displays the vertex count in the status bar.
    *
    * @param[in] mainWindow         The window which contains the status bar to be updated.
    * @param[in] message            The message to be set on the status bar.
    */
   void SetStatusBarMessage(const MainWindow& mainWindow, const std::wstring& message)
   {
      mainWindow.statusBar()->showMessage(QString::fromStdWString(message));
   }

   /**
    * @brief ScanAndParse
    * @param treeMap
    * @param mainWindow
    */
   void ScanAndParse(Visualization& treeMap, const MainWindow& mainWindow)
   {
      const auto statusBarUpdater = [&] (const std::uintmax_t numberOfFilesScanned)
      {
         std::wstringstream message;
         message.imbue(std::locale(""));
         message << std::fixed << L"Files Scanned: " << numberOfFilesScanned;
         SetStatusBarMessage(mainWindow, message.str());
      };

      treeMap.ScanDirectory(statusBarUpdater);
      treeMap.ParseScan();
   }
}

GLCanvas::GLCanvas(QWidget* parent)
   : QOpenGLWidget(parent),
     m_treeMap(nullptr),
     m_isPaintingSuspended(false),
     m_isVisualizationLoaded(false),
     m_mainWindow(reinterpret_cast<MainWindow*>(parent)),
     m_distance(2.5),
     m_cameraMovementSpeed(0.25),
     m_mouseSensitivity(0.25),
     m_lastFrameTimeStamp(std::chrono::system_clock::now()),
     m_visualizationVertexColorBuffer(QOpenGLBuffer::VertexBuffer),
     m_visualizationVertexPositionBuffer(QOpenGLBuffer::VertexBuffer)
{
   if (!m_mainWindow)
   {
      throw std::invalid_argument("Parent couldn't be interpreted as a MainWindow instance.");
   }

   // Set up the camera:
   m_camera.SetAspectRatio(3.0f / 2.0f);
   m_camera.SetPosition(QVector3D(500, 100, 0));
   //m_camera.LookAt(QVector3D(500, 0, -500));

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

void GLCanvas::ParseVisualization(const std::wstring& path, const ParsingOptions& options)
{
   if (path.empty())
   {
      return;
   }

   m_isPaintingSuspended = true;

   m_visualizedDirectory = path;

   if (!m_treeMap || options.forceNewScan)
   {
      m_treeMap.reset(new SquarifiedTreeMap(m_visualizedDirectory));
   }

   if (m_treeMap && (options.forceNewScan || !m_treeMap->HasScanBeenPerformed()))
   {
      ScanAndParse(*m_treeMap, *m_mainWindow);
   }

   m_visualizationVertices = m_treeMap->PopulateVertexBuffer(options);
   m_visualizationColors = m_treeMap->PopulateColorBuffer(options);

   m_isVisualizationLoaded = !(m_visualizationVertices.empty() && m_visualizationColors.empty());

   if (m_isVisualizationLoaded)
   {
      m_visualizationShaderProgram.removeAllShaders();
      m_visualizationVAO.destroy();

      PrepareVisualizationVertexBuffers();
      PrepareVisualizationShaderProgram();
   }

   std::wstringstream message;
   message.imbue(std::locale(""));
   message << std::fixed << m_treeMap->GetVertexCount() << L" vertices in "
      << (m_treeMap->GetVertexCount() / Block::VERTICES_PER_BLOCK) << L" blocks";
   SetStatusBarMessage(*m_mainWindow, message.str());

   m_isPaintingSuspended = false;
}

void GLCanvas::SetFieldOfView(const float fieldOfView)
{
   m_camera.SetFieldOfView(fieldOfView);
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

void GLCanvas::initializeGL()
{
   initializeOpenGLFunctions();

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);

   PrepareVisualizationShaderProgram();
   PrepareOriginMarkerShaderProgram();

   PrepareVisualizationVertexBuffers();
   PrepareOriginMarkerVertexBuffers();

   m_light = Light(QVector3D(2, 2, 0), QVector3D(1, 1, 1), 0.01f, 0.75f);
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
      m_cameraMovementSpeed *= 1.25f;
   }
   else if (event->key() == Qt::Key_Down)
   {
      m_cameraMovementSpeed *= 0.75f;
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
   const float deltaX = event->x() - m_lastMousePosition.x();
   const float deltaY = event->y() - m_lastMousePosition.y();

   if (event->buttons() & Qt::LeftButton)
   {
      m_camera.OffsetOrientation(m_mouseSensitivity * deltaY, m_mouseSensitivity * deltaX);
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

      m_mainWindow->UpdateFieldOfViewSlider(static_cast<float>(m_camera.GetFieldOfView()));
   }

   event->accept();
}

void GLCanvas::OnCameraMovementSpeedChanged(const double newSpeed)
{
   m_cameraMovementSpeed = newSpeed;
}

void GLCanvas::OnMouseSensitivityChanged(const double newSensitivity)
{
   m_mouseSensitivity = newSensitivity;
}

void GLCanvas::HandleCameraMovement()
{
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
      m_camera.OffsetPosition(millisecondsElapsed.count() * m_cameraMovementSpeed * m_camera.Forward());
   }

   if (isKeyADown)
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * m_cameraMovementSpeed * m_camera.Left());
   }

   if (isKeySDown)
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * m_cameraMovementSpeed * m_camera.Backward());
   }

   if (isKeyDDown)
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * m_cameraMovementSpeed * m_camera.Right());
   }

   //m_light.SetPosition(QVector3D(500, 150, -500));
   m_light.SetPosition(m_camera.GetPosition());
}

void GLCanvas::paintGL()
{
   if (m_isPaintingSuspended)
   {
      return;
   }

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   const auto currentTime = std::chrono::system_clock::now();
   auto millisecondsElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now() - m_lastFrameTimeStamp).count();

   if (millisecondsElapsed == 0)
   {
      millisecondsElapsed = 1;
   }

   if (m_mainWindow)
   {
      m_mainWindow->setWindowTitle(QString::fromStdString("D-Viz @ ") +
         QString::number((1000 / millisecondsElapsed)) + QString::fromStdString(" fps [*]"));
   }

   HandleCameraMovement();

   // Draw origin marker:
   m_originMarkerShaderProgram.bind();
   m_originMarkerShaderProgram.setUniformValue("mvpMatrix", m_camera.GetMatrix());

   m_originMarkerVAO.bind();

   glDrawArrays(GL_LINES, /* first = */ 0, /* count = */ m_originMarkerVertices.size());

   m_originMarkerShaderProgram.release();
   m_originMarkerVAO.release();

   if (m_isVisualizationLoaded)
   {
      // Draw visualization:
      m_visualizationShaderProgram.bind();

      // The model matrix is always the same, since the model doesn't move:
      m_visualizationShaderProgram.setUniformValue("model", QMatrix4x4());
      m_visualizationShaderProgram.setUniformValue("mvpMatrix", m_camera.GetMatrix());
      m_visualizationShaderProgram.setUniformValue("cameraPosition", m_camera.GetPosition());

      m_visualizationShaderProgram.setUniformValue("materialShininess", 80.0f);
      m_visualizationShaderProgram.setUniformValue("materialSpecularColor", QVector3D(1, 1, 1));

      m_visualizationShaderProgram.setUniformValue("light.position", m_light.position);
      m_visualizationShaderProgram.setUniformValue("light.intensity", m_light.intensity);
      m_visualizationShaderProgram.setUniformValue("light.attenuation", 0.02f);
      m_visualizationShaderProgram.setUniformValue("light.ambientCoefficient", 0.005f);

      m_visualizationVAO.bind();

      glDrawArrays(GL_TRIANGLES, /* first = */ 0, /* count = */ m_visualizationVertices.size());

      m_visualizationShaderProgram.release();
      m_visualizationVAO.release();
   }

   m_lastFrameTimeStamp = currentTime;
}
