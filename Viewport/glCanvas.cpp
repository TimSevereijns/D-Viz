#include "glCanvas.h"

#include "camera.h"
#include "DriveScanner/driveScanner.h"
#include "mainwindow.h"
#include "optionsManager.h"
#include "Scene/debuggingRayAsset.h"
#include "Scene/gridAsset.h"
#include "Scene/selectionHighlightAsset.h"
#include "Scene/visualizationAsset.h"
#include "Visualizations/squarifiedTreemap.h"
#include "Utilities/scopeExit.hpp"

#include <QApplication>
#include <QMouseEvent>
#include <QOpenGLShader>
#include <QStatusBar>
#include <QTimer>

#include <iostream>
#include <sstream>
#include <stdexcept>

namespace
{
   /**
    * @brief keyPressHelper is a helper function to update the press state of a keyboard key.
    *
    * @param[in] keyboardManager        The keyboard manager on which to update the key state.
    * @param[in] key                    The QKeyEvent that triggered the call.
    * @param[in] state                  Whether the key in question is up (released) or down
    *                                  (pressed).
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
    * @brief SetStatusBarMessage displays the vertex count in the status bar.
    *
    * @param[in] mainWindow         The window which contains the status bar to be updated.
    * @param[in] message            The message to be set on the status bar.
    */
   inline void SetStatusBarMessage(const MainWindow& mainWindow, const std::wstring& message)
   {
      mainWindow.statusBar()->showMessage(QString::fromStdWString(message));
   }

   /**
    * @brief UpdateVertexCountInStatusBar
    * @param vertexCount
    * @param mainWindow
    */
   void UpdateVertexCountInStatusBar(const unsigned int vertexCount, const MainWindow& mainWindow)
   {
      std::wstringstream message;
      message.imbue(std::locale(""));
      message << std::fixed << vertexCount << L" vertices in "
         << (vertexCount / Block::VERTICES_PER_BLOCK) << L" blocks";

      SetStatusBarMessage(mainWindow, message.str());
   }

   /**
    * @brief GetFullNodePath
    *
    * @todo Fix printing of complete path.
    *
    * @param[in] node               The selected node.
    *
    * @returns the complete path as created by following the current node up to the root.
    */
   std::wstring GetFullNodePath(const TreeNode<VizNode>& node)
   {
      std::vector<std::wstring> reversePath;
      reversePath.reserve(Tree<VizNode>::Depth(node));
      reversePath.emplace_back(node->file.name);

      TreeNode<VizNode> currentNode = node;

      while (currentNode.GetParent())
      {
         currentNode = *currentNode.GetParent();
         reversePath.emplace_back(currentNode->file.name);
      }

      return std::accumulate(std::rbegin(reversePath) + 1, std::rend(reversePath), std::wstring(),
         [] (const std::wstring path, const std::wstring file)
      {
         return path + std::wstring(L"/") + file;
      });
   }

   /**
    * @brief HighlightSelectedFile will outline the selected node.
    *
    * @param[in] node               The selected node.
    * @param[in] highlightAsset     The visual asset representing the outline.
    * @param[in] camera             The camera from which the selection was made.
    */
   void HighlightSelection(const TreeNode<VizNode>& node, SceneAsset& highlightAsset,
      const Camera& camera)
   {
      std::wcout << GetFullNodePath(node) << std::endl;
      //std::wcout << L"Bounding Box: " << node->boundingBox.height << std::endl;

      QVector<QVector3D> nodeVertices;

      std::for_each(std::begin(node->block), std::end(node->block),
         [&] (const BlockFace& face)
      {
         nodeVertices << face.vertices;
      });

      QVector<QVector3D> vertices;
      QVector<QVector3D> colors;

      vertices
         // Top:
         << nodeVertices[48] << nodeVertices[50]
         << nodeVertices[50] << nodeVertices[54]
         << nodeVertices[54] << nodeVertices[56]
         << nodeVertices[56] << nodeVertices[48]
         // Bottom:
         << nodeVertices[ 0] << nodeVertices[ 2]
         << nodeVertices[ 2] << nodeVertices[14]
         << nodeVertices[14] << nodeVertices[26]
         << nodeVertices[26] << nodeVertices[ 0]
         // Sides:
         << nodeVertices[ 0] << nodeVertices[ 4]
         << nodeVertices[ 2] << nodeVertices[ 6]
         << nodeVertices[26] << nodeVertices[30]
         << nodeVertices[24] << nodeVertices[28];

      const QVector3D hotPink = QVector3D { 1.0f, 105.0f / 255.0f, 180.0f / 255.0f };
      for (int index = 0; index < vertices.size(); index++)
      {
         colors << hotPink;
      }

      highlightAsset.SetVertexData(std::move(vertices));
      highlightAsset.SetColorData(std::move(colors));
      highlightAsset.Reload(camera);
   }

   /**
    * @brief The Asset enum
    */
   enum Asset
   {
      GRID = 0,      ///< GridAsset
      TREEMAP,       ///< VisualizationAsset
      PICKING_RAY,   ///< DebuggingRayAsset
      HIGHLIGHT      ///< SelectionHighlightAsset
   };
}

GLCanvas::GLCanvas(QWidget* parent)
   : QOpenGLWidget(parent),
     m_graphicsDevice(nullptr),
     m_theVisualization(nullptr),
     m_isPaintingSuspended(false),
     m_isVisualizationLoaded(false),
     m_mainWindow(reinterpret_cast<MainWindow*>(parent)),
     m_lastFrameTimeStamp(std::chrono::system_clock::now())
{
   if (!m_mainWindow)
   {
      throw std::invalid_argument("Parent couldn't be interpreted as a MainWindow instance.");
   }

   m_settings = m_mainWindow->GetOptionsManager();

   // Set up the camera:
   m_camera.SetPosition(QVector3D(500, 100, 0));

   // Set keyboard and mouse focus:
   setFocusPolicy(Qt::StrongFocus);

   QSurfaceFormat format;
   format.setDepthBufferSize(24);
   format.setSamples(8);
   setFormat(format);

   // Set the target frame rate:
   m_frameRedrawTimer.reset(new QTimer(this));
   connect(m_frameRedrawTimer.get(), SIGNAL(timeout()), this, SLOT(update()));
   m_frameRedrawTimer->start(20);
}

void GLCanvas::initializeGL()
{
   m_graphicsDevice = std::make_unique<GraphicsDevice>();

   m_graphicsDevice->glEnable(GL_DEPTH_TEST);
   m_graphicsDevice->glEnable(GL_CULL_FACE);
   m_graphicsDevice->glEnable(GL_MULTISAMPLE);
   m_graphicsDevice->glEnable(GL_LINE_SMOOTH);

   m_sceneAssets.emplace_back(std::make_unique<GridAsset>(*m_graphicsDevice));
   m_sceneAssets.emplace_back(std::make_unique<VisualizationAsset>(*m_graphicsDevice));
   m_sceneAssets.emplace_back(std::make_unique<DebuggingRayAsset>(*m_graphicsDevice));
   m_sceneAssets.emplace_back(std::make_unique<SelectionHighlightAsset>(*m_graphicsDevice));

   for (const auto& asset : m_sceneAssets)
   {
      asset->LoadShaders();

      asset->PrepareVertexBuffers(m_camera);
      asset->PrepareColorBuffers(m_camera);
   }
}

void GLCanvas::resizeGL(int width, int height)
{
   // Avoid a divide-by-zero situation:
   if (height == 0)
   {
      height = 1;
   }

   m_graphicsDevice->glViewport(0, 0, width, height);

   m_camera.SetAspectRatio(static_cast<float>(width) / static_cast<float>(height));
   m_camera.SetViewport(QRect(QPoint(0,0), QPoint(width, height)));
}


void GLCanvas::CreateNewVisualization(const VisualizationParameters& parameters)
{
   if (parameters.rootDirectory.empty())
   {
      return;
   }

   if (!m_theVisualization || parameters.forceNewScan)
   {
      m_theVisualization.reset(new SquarifiedTreeMap(parameters));
      ScanDrive(parameters);
   }
}

void GLCanvas::ScanDrive(const VisualizationParameters& vizParameters)
{
   const auto& progressHandler =
      [&] (const std::uintmax_t numberOfFilesScanned)
   {
      std::wstringstream message;
      message.imbue(std::locale(""));
      message << std::fixed << L"Files Scanned: " << numberOfFilesScanned;
      SetStatusBarMessage(*m_mainWindow, message.str());
   };

   const auto& completionHandler =
      [&, vizParameters] (const std::uintmax_t numberOfFilesScanned)
   {
      const bool previousSuspensionState = m_isPaintingSuspended;
      m_isPaintingSuspended = true;

      ON_SCOPE_EXIT(m_isPaintingSuspended = previousSuspensionState);

      std::wstringstream message;
      message.imbue(std::locale(""));
      message << std::fixed << L"Total Files Scanned: " << numberOfFilesScanned;
      SetStatusBarMessage(*m_mainWindow, message.str());

      const auto& theTree = m_scanner.GetTree();
      m_theVisualization->Parse(theTree);
      m_theVisualization->UpdateBoundingBoxes();

      ReloadVisualization(vizParameters);
   };

   DriveScannerParameters scanningParameters;
   scanningParameters.onProgressUpdateCallback = progressHandler;
   scanningParameters.onScanCompletedCallback = completionHandler;
   scanningParameters.path = vizParameters.rootDirectory;

   m_scanner.SetParameters(scanningParameters);
   m_scanner.StartScanning();
}

void GLCanvas::ReloadVisualization(const VisualizationParameters& parameters)
{
   const bool previousSuspensionState = m_isPaintingSuspended;
   m_isPaintingSuspended = true;

   ON_SCOPE_EXIT(m_isPaintingSuspended = previousSuspensionState);

   m_visualizationParameters = parameters;
   m_theVisualization->ComputeVertexAndColorData(parameters);

   m_sceneAssets[Asset::TREEMAP]->SetVertexData(std::move(m_theVisualization->GetVertexData()));
   m_sceneAssets[Asset::TREEMAP]->SetColorData(std::move(m_theVisualization->GetColorData()));

   m_isVisualizationLoaded = m_sceneAssets[Asset::TREEMAP]->IsAssetLoaded();

   if (m_isVisualizationLoaded)
   {
      for (const auto& asset : m_sceneAssets)
      {
         asset->Reload(m_camera);
      }
   }

   UpdateVertexCountInStatusBar(m_sceneAssets[Asset::TREEMAP]->GetVertexCount(), *m_mainWindow);
}

void GLCanvas::SetFieldOfView(const float fieldOfView)
{
   m_camera.SetFieldOfView(fieldOfView);
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
      m_settings->m_cameraMovementSpeed *= 1.25f;
   }
   else if (event->key() == Qt::Key_Down)
   {
      m_settings->m_cameraMovementSpeed *= 0.75f;
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

void GLCanvas::HandleRightClick(const QMouseEvent& event)
{
   if (!m_isVisualizationLoaded)
   {
      return;
   }

   using namespace std::chrono;
   const auto startTime = system_clock::now();

   const auto canvasCoordinates = QPoint(event.x(), event.y());
   const auto ray = m_camera.ShootRayIntoScene(canvasCoordinates);
   const auto selection = m_theVisualization->FindNearestIntersection(ray, m_visualizationParameters);
   if (selection)
   {
      HighlightSelection(*selection, *m_sceneAssets[Asset::HIGHLIGHT], m_camera);
   }

   const auto endTime = system_clock::now();
   const auto selectionTime = duration_cast<std::chrono::milliseconds>(endTime - startTime);

   std::cout << "Node selected in time: " << selectionTime.count() << "ms" << std::endl;
}

void GLCanvas::mousePressEvent(QMouseEvent* const event)
{
   m_lastMousePosition = event->pos();

   if (event->button() == Qt::RightButton)
   {
      HandleRightClick(*event);
   }

   event->accept();
}

void GLCanvas::mouseMoveEvent(QMouseEvent* const event)
{
   const float deltaX = event->x() - m_lastMousePosition.x();
   const float deltaY = event->y() - m_lastMousePosition.y();

   if (event->buttons() & Qt::LeftButton)
   {
      m_camera.OffsetOrientation(m_settings->m_mouseSensitivity * deltaY,
         m_settings->m_mouseSensitivity * deltaX);
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

void GLCanvas::HandleInput()
{
   if (m_settings->m_useXBoxController && m_mainWindow->IsXboxControllerConnected())
   {
      HandleXBoxControllerInput();

      return;
   }

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
      m_camera.OffsetPosition(millisecondsElapsed.count() * m_settings->m_cameraMovementSpeed *
         m_camera.Forward());
   }

   if (isKeyADown)
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * m_settings->m_cameraMovementSpeed *
         m_camera.Left());
   }

   if (isKeySDown)
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * m_settings->m_cameraMovementSpeed *
         m_camera.Backward());
   }

   if (isKeyDDown)
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * m_settings->m_cameraMovementSpeed *
         m_camera.Right());
   }
}

void GLCanvas::HandleXBoxControllerInput()
{
   static const int MOVEMENT_AMPLIFICATION_FACTOR = 8;

   const auto millisecondsElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now() - m_lastFrameTimeStamp);

   const XboxController::State& controllerState = m_mainWindow->GetXboxControllerState();
   const XboxController& controller = m_mainWindow->GetXboxControllerManager();

   if (controller.IsButtonDown(XINPUT_GAMEPAD_DPAD_UP))
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * m_settings->m_cameraMovementSpeed
         * m_camera.Forward());
   }

   if (controller.IsButtonDown(XINPUT_GAMEPAD_DPAD_LEFT))
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * m_settings->m_cameraMovementSpeed
         * m_camera.Left());
   }

   if (controller.IsButtonDown(XINPUT_GAMEPAD_DPAD_DOWN))
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * m_settings->m_cameraMovementSpeed
         * m_camera.Backward());
   }

   if (controller.IsButtonDown(XINPUT_GAMEPAD_DPAD_RIGHT))
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * m_settings->m_cameraMovementSpeed
         * m_camera.Right());
   }

   if (controller.IsButtonDown(XINPUT_GAMEPAD_LEFT_SHOULDER))
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() *
         (m_settings->m_cameraMovementSpeed / MOVEMENT_AMPLIFICATION_FACTOR) * m_camera.Down());
   }

   if (controller.IsButtonDown(XINPUT_GAMEPAD_RIGHT_SHOULDER))
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() *
         (m_settings->m_cameraMovementSpeed / MOVEMENT_AMPLIFICATION_FACTOR) * m_camera.Up());
   }

   // Handle camera orientation via right thumb stick:
   if (controllerState.rightThumbX || controllerState.rightThumbY)
   {
      m_camera.OffsetOrientation(
         MOVEMENT_AMPLIFICATION_FACTOR * m_settings->m_mouseSensitivity * -controllerState.rightThumbY,
         MOVEMENT_AMPLIFICATION_FACTOR * m_settings->m_mouseSensitivity * controllerState.rightThumbX);
   }

   // Handle camera forward/backward movement via left thumb stick:
   if (controllerState.leftThumbY)
   {
      m_camera.OffsetPosition(
         MOVEMENT_AMPLIFICATION_FACTOR * m_settings->m_cameraMovementSpeed * controllerState.leftThumbY
         * m_camera.Forward());
   }

   // Handle camera left/right movement via left thumb stick:
   if (controllerState.leftThumbX)
   {
      m_camera.OffsetPosition(
         MOVEMENT_AMPLIFICATION_FACTOR * m_settings->m_cameraMovementSpeed * controllerState.leftThumbX
         * m_camera.Right());
   }
}

void GLCanvas::UpdateFPS()
{
   using namespace std::chrono;

   const auto currentTime = system_clock::now();
   const auto millisecondsElapsed = std::max<unsigned int>(
      duration_cast<milliseconds>(currentTime - m_lastFrameTimeStamp).count(),
      1); // This will avoid division by zero.

   m_lastFrameTimeStamp = currentTime;

   if (m_frameRateDeque.size() > 32)
   {
      m_frameRateDeque.pop_front();
   }

   m_frameRateDeque.emplace_back(1000 / millisecondsElapsed);

   const int fpsSum = std::accumulate(std::begin(m_frameRateDeque), std::end(m_frameRateDeque), 0,
      [] (const int currentTotal, const int fps)
   {
      return currentTotal + fps;
   });

   const auto averageFps = fpsSum / m_frameRateDeque.size();

   if (m_mainWindow)
   {
      m_mainWindow->setWindowTitle(
         QString::fromStdString("D-Viz @ ") +
         QString::number(averageFps) +
         QString::fromStdString(" fps [*]"));
   }
}

void GLCanvas::paintGL()
{
   if (m_isPaintingSuspended)
   {
      return;
   }

   HandleInput();
   UpdateFPS();

   if (m_settings->m_isLightAttachedToCamera)
   {
      m_light.position = m_camera.GetPosition();
   }

   m_graphicsDevice->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   for (const auto& asset : m_sceneAssets)
   {
      asset->Render(m_camera, m_light, *m_settings);
   }
}