#include "glCanvas.h"

#include "../constants.h"

#include "Scene/crosshairAsset.h"
#include "Scene/debuggingRayAsset.h"
#include "Scene/gridAsset.h"
#include "Scene/visualizationAsset.h"

#include "canvasContextMenu.h"
#include "Utilities/scopeExit.hpp"
#include "Visualizations/squarifiedTreemap.h"

#include <QApplication>
#include <QMenu>
#include <QMessageBox>

#include <iostream>

namespace
{
   /**
    * @brief Provides an easier way to index into the asset vector, than memorizing indices.
    */
   enum Asset
   {
      GRID = 0,      ///< GridAsset
      TREEMAP,       ///< VisualizationAsset
      CROSSHAIR      ///< NodeSelectionCrosshair
   };
}

GLCanvas::GLCanvas(
   Controller& controller,
   QWidget* parent)
   :
   QOpenGLWidget{ parent },
   m_controller{ controller },
   m_mainWindow{ *(reinterpret_cast<MainWindow*>(parent)) }
{
   m_optionsManager = m_mainWindow.GetOptionsManager();

   m_camera.SetPosition(QVector3D{ 500, 100, 0 });

   setFocusPolicy(Qt::StrongFocus);

   QSurfaceFormat format;
   format.setDepthBufferSize(24);
   format.setSamples(8);
   format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
   setFormat(format);

   m_frameRedrawTimer.reset(new QTimer{ this });
   connect(m_frameRedrawTimer.get(), SIGNAL(timeout()), this, SLOT(update()));
   m_frameRedrawTimer->start(Constants::Graphics::DESIRED_TIME_BETWEEN_FRAMES);

   m_inputCaptureTimer.reset(new QTimer{ this });
   connect(m_inputCaptureTimer.get(), SIGNAL(timeout()), this, SLOT(HandleInput()));
   m_inputCaptureTimer->start(Constants::Graphics::DESIRED_TIME_BETWEEN_FRAMES);
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
   m_sceneAssets.emplace_back(std::make_unique<CrosshairAsset>(*m_graphicsDevice));

   for (const auto& asset : m_sceneAssets)
   {
      asset->LoadShaders();
      asset->Initialize();
   }
}

void GLCanvas::resizeGL(int width, int height)
{
   if (height == 0)
   {
      height = 1;
   }

   m_graphicsDevice->glViewport(0, 0, width, height);

   m_camera.SetViewport(QRect{ QPoint{ 0, 0 }, QPoint{ width, height } });
}

void GLCanvas::ReloadVisualization()
{
   const bool previousSuspensionState = m_isPaintingSuspended;
   m_isPaintingSuspended = true;
   ON_SCOPE_EXIT noexcept { m_isPaintingSuspended = previousSuspensionState; };

   auto* const vizAsset = dynamic_cast<VisualizationAsset*>(m_sceneAssets[Asset::TREEMAP].get());
   assert(vizAsset);

   const auto parameters = m_controller.GetVisualizationParameters();
   const auto blockCount = vizAsset->LoadBufferData(m_controller.GetTree(), parameters);

   for (const auto& asset : m_sceneAssets)
   {
      asset->Reload();
   }

   assert (blockCount == vizAsset->GetBlockCount());
   m_controller.PrintMetadataToStatusBar(blockCount);
}

void GLCanvas::SetFieldOfView(const float fieldOfView)
{
   m_camera.SetFieldOfView(fieldOfView);
}

void GLCanvas::keyPressEvent(QKeyEvent* const event)
{
   assert(event);
   if (!event)
   {
      return;
   }

   if (event->isAutoRepeat())
   {
      event->ignore();
      return;
   }

   const auto state = KeyboardManager::KEY_STATE::DOWN;
   m_keyboardManager.UpdateKeyState(static_cast<Qt::Key>(event->key()), state);

   event->accept();
}

void GLCanvas::keyReleaseEvent(QKeyEvent* const event)
{
   assert(event);
   if (!event)
   {
      return;
   }

   if (event->isAutoRepeat())
   {
      event->ignore();
      return;
   }

   const auto state = KeyboardManager::KEY_STATE::UP;
   m_keyboardManager.UpdateKeyState(static_cast<Qt::Key>(event->key()), state);

   event->accept();
}

void GLCanvas::mousePressEvent(QMouseEvent* const event)
{
   assert(event);
   if (!event)
   {
      return;
   }

   m_lastMousePosition = event->pos();

   if (event->button() == Qt::RightButton)
   {
      if (m_keyboardManager.IsKeyDown(Qt::Key_Control))
      {
         ShowContextMenu(m_lastMousePosition);
      }
      else
      {
         const auto ray = m_camera.ShootRayIntoScene(event->pos());
         m_controller.SelectNodeViaRay(m_camera, ray);
      }
   }
   else if (event->button() == Qt::LeftButton)
   {
      if (!m_isLeftMouseButtonDown)
      {
         m_isLeftMouseButtonDown = true;
         m_startOfMouseLookEvent = std::chrono::system_clock::now();
      }
   }

   event->accept();
}

void GLCanvas::mouseReleaseEvent(QMouseEvent* const event)
{
   assert(event);
   if (!event)
   {
      return;
   }

   if (event->button() == Qt::LeftButton)
   {
      m_isLeftMouseButtonDown = false;

      if (m_isCursorHidden)
      {
         const auto globalCursorPosition = mapToGlobal(m_camera.GetViewport().center());
         cursor().setPos(globalCursorPosition.x(), globalCursorPosition.y());
      }

      setCursor(Qt::ArrowCursor);
      m_isCursorHidden = false;
   }
}

void GLCanvas::mouseMoveEvent(QMouseEvent* const event)
{
   assert(event);
   if (!event)
   {
      return;
   }

   const float deltaX = event->x() - m_lastMousePosition.x();
   const float deltaY = event->y() - m_lastMousePosition.y();

   if (!m_isCursorHidden)
   {
      m_lastMousePosition = event->pos();
   }

   if (event->buttons() & Qt::LeftButton)
   {
      const auto now = std::chrono::system_clock::now();
      const auto timeSinceStartOfLookEvent =
         std::chrono::duration_cast<std::chrono::seconds>(now - m_startOfMouseLookEvent);

      if (timeSinceStartOfLookEvent >= std::chrono::seconds{ 2 })
      {
         setCursor(Qt::BlankCursor);
         m_isCursorHidden = true;

         // In order to correctly set the cursor's position, we need to use coordinates that are
         // relative to the virtual monitor. However, in order to correctly process mouse movements
         // within this class, we need to store the cursor's position relative to the widget's
         // coordinate system.

         const auto cursorPositionOnCanvas = m_camera.GetViewport().center();
         const auto cursorPositionOnMonitor = mapToGlobal(cursorPositionOnCanvas);
         cursor().setPos(cursorPositionOnMonitor.x(), cursorPositionOnMonitor.y());

         m_lastMousePosition = cursorPositionOnCanvas;
      }

      m_camera.OffsetOrientation(
         m_optionsManager->m_mouseSensitivity * deltaY,
         m_optionsManager->m_mouseSensitivity * deltaX);
   }

   event->accept();
}

void GLCanvas::wheelEvent(QWheelEvent* const event)
{
   assert(event);
   if (!event)
   {
      return;
   }

   event->accept();

   if (event->orientation() != Qt::Vertical)
   {
      return;
   }

   const int delta = event->delta();

   if (m_keyboardManager.IsKeyUp(Qt::Key_Shift))
   {
      if (delta > 0 && m_optionsManager->m_cameraMovementSpeed < 1.0)
      {
         m_optionsManager->m_cameraMovementSpeed += 0.01;
      }
      else if (delta < 0 && m_optionsManager->m_cameraMovementSpeed > 0.01)
      {
         m_optionsManager->m_cameraMovementSpeed -= 0.01;
      }

      m_mainWindow.SetCameraSpeedSpinner(static_cast<double>(m_optionsManager->m_cameraMovementSpeed));
   }
   else
   {
      if (delta < 0)
      {
        m_camera.IncreaseFieldOfView();
      }
      else if (delta > 0)
      {
         m_camera.DecreaseFieldOfView();
      }

      m_mainWindow.SetFieldOfViewSlider(static_cast<float>(m_camera.GetFieldOfView()));
   }
}

void GLCanvas::SelectNode(const TreeNode<VizNode>* const node)
{
   m_sceneAssets[Asset::TREEMAP]->UpdateVBO(
      *node,
      SceneAsset::UpdateAction::SELECT,
      m_controller.GetVisualizationParameters());
}

void GLCanvas::RestoreSelectedNode()
{
   if (m_controller.GetSelectedNode())
   {
      m_sceneAssets[Asset::TREEMAP]->UpdateVBO(
         *m_controller.GetSelectedNode(),
         SceneAsset::UpdateAction::DESELECT,
         m_controller.GetVisualizationParameters());
   }
}

void GLCanvas::HighlightSelectedNodes(std::vector<const TreeNode<VizNode>*>& nodes)
{
   for (const auto* const node : nodes)
   {
      SelectNode(node);
   }
}

void GLCanvas::RestoreHighlightedNodes(std::vector<const TreeNode<VizNode>*>& nodes)
{
   for (const auto* const node : nodes)
   {
      m_sceneAssets[Asset::TREEMAP]->UpdateVBO(
         *node,
         SceneAsset::UpdateAction::DESELECT,
         m_controller.GetVisualizationParameters());
   }
}

void GLCanvas::ShowContextMenu(const QPoint& point)
{
   const auto* const selectedNode = m_controller.GetSelectedNode();
   if (!selectedNode)
   {
      return;
   }

   const auto highlightSelectionCallback = [&] (std::vector<const TreeNode<VizNode>*>& nodes)
   {
      HighlightSelectedNodes(nodes);
   };

   CanvasContextMenu menu{ m_keyboardManager };

   menu.addAction("Highlight Ancestors", [&]
   {
      m_controller.HighlightAncestors(*selectedNode, highlightSelectionCallback);
   });

   menu.addAction("Highlight Descendants", [&]
   {
      m_controller.HighlightDescendants(*selectedNode, highlightSelectionCallback);
   });

   if (selectedNode->GetData().file.type == FileType::REGULAR)
   {
      const auto entryText =
         QString::fromStdWString(L"Highlight All ")
         + QString::fromStdWString(selectedNode->GetData().file.extension)
         + QString::fromStdWString(L" Files");

      menu.addAction(entryText, [&]
      {
         m_controller.HighlightAllMatchingExtensions(*selectedNode, highlightSelectionCallback);
      });
   }

   menu.addSeparator();
   menu.addAction("Show in Explorer", [&]
   {
      Controller::ShowInFileExplorer(*selectedNode);
   });

   const QPoint globalPoint = mapToGlobal(point);
   menu.exec(globalPoint);
}

void GLCanvas::HandleInput()
{
   assert(m_optionsManager);

   const auto now = std::chrono::system_clock::now();
   ON_SCOPE_EXIT noexcept { m_lastCameraPositionUpdatelTime = now; };

   if (m_optionsManager->m_useXBoxController && m_mainWindow.IsXboxControllerConnected())
   {
      HandleXBoxControllerInput();

       return;
   }

   const auto millisecondsElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      now - m_lastCameraPositionUpdatelTime);

   const bool isWKeyDown = m_keyboardManager.IsKeyDown(Qt::Key_W);
   const bool isAKeyDown = m_keyboardManager.IsKeyDown(Qt::Key_A);
   const bool isSKeyDown = m_keyboardManager.IsKeyDown(Qt::Key_S);
   const bool isDKeyDown = m_keyboardManager.IsKeyDown(Qt::Key_D);

   if ((isWKeyDown && isSKeyDown) || (isAKeyDown && isDKeyDown))
   {
      return;
   }

   const auto cameraSpeed = m_optionsManager->m_cameraMovementSpeed;

   if (isWKeyDown)
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * cameraSpeed * m_camera.Forward());
   }

   if (isAKeyDown)
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * cameraSpeed * m_camera.Left());
   }

   if (isSKeyDown)
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * cameraSpeed * m_camera.Backward());
   }

   if (isDKeyDown)
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * cameraSpeed * m_camera.Right());
   }
}

void GLCanvas::HandleXBoxControllerInput()
{
   const auto millisecondsElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now() - m_lastCameraPositionUpdatelTime);

   const XboxController::State& controllerState = m_mainWindow.GetXboxControllerState();
   const XboxController& controller = m_mainWindow.GetXboxControllerManager();

   const auto cameraSpeed =
       m_optionsManager->m_cameraMovementSpeed / Constants::Xbox::MOVEMENT_AMPLIFICATION;

   if (controller.IsButtonDown(XINPUT_GAMEPAD_DPAD_UP))
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * cameraSpeed * m_camera.Forward());
   }

   if (controller.IsButtonDown(XINPUT_GAMEPAD_DPAD_LEFT))
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * cameraSpeed * m_camera.Left());
   }

   if (controller.IsButtonDown(XINPUT_GAMEPAD_DPAD_DOWN))
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * cameraSpeed * m_camera.Backward());
   }

   if (controller.IsButtonDown(XINPUT_GAMEPAD_DPAD_RIGHT))
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * cameraSpeed * m_camera.Right());
   }

   if (controller.IsButtonDown(XINPUT_GAMEPAD_LEFT_SHOULDER))
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * cameraSpeed * m_camera.Down());
   }

   if (controller.IsButtonDown(XINPUT_GAMEPAD_RIGHT_SHOULDER))
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * cameraSpeed * m_camera.Up());
   }

   HandleXboxThumbstickInput(controllerState);
   HandleXboxTriggerInput(controllerState);
}

void GLCanvas::HandleXboxThumbstickInput(const XboxController::State& controllerState)
{
   if (controllerState.rightThumbX || controllerState.rightThumbY)
   {
      const auto pitch =
         Constants::Xbox::MOVEMENT_AMPLIFICATION
         * m_optionsManager->m_mouseSensitivity
         * -controllerState.rightThumbY;

      const auto yaw =
         Constants::Xbox::MOVEMENT_AMPLIFICATION
         * m_optionsManager->m_mouseSensitivity
         * controllerState.rightThumbX;

      m_camera.OffsetOrientation(pitch, yaw);
   }

   if (controllerState.leftThumbY)
   {
      m_camera.OffsetPosition(
         Constants::Xbox::MOVEMENT_AMPLIFICATION
         * m_optionsManager->m_cameraMovementSpeed
         * controllerState.leftThumbY
         * m_camera.Forward());
   }

   if (controllerState.leftThumbX)
   {
      m_camera.OffsetPosition(
         Constants::Xbox::MOVEMENT_AMPLIFICATION
         * m_optionsManager->m_cameraMovementSpeed
         * controllerState.leftThumbX
         * m_camera.Right());
   }
}

void GLCanvas::HandleXboxTriggerInput(const XboxController::State& controllerState)
{
   if (!m_isLeftTriggerDown
       && controllerState.leftTrigger > Constants::Xbox::TRIGGER_ACTUATION_THRESHOLD)
   {
      m_isLeftTriggerDown = true;

      auto* const crosshairAsset =
         dynamic_cast<CrosshairAsset*>(m_sceneAssets[Asset::CROSSHAIR].get());

      assert(crosshairAsset);
      if (crosshairAsset)
      {
         crosshairAsset->Show(m_camera);
      }
   }
   else if (m_isLeftTriggerDown
      && controllerState.leftTrigger <= Constants::Xbox::TRIGGER_ACTUATION_THRESHOLD)
   {
      m_isLeftTriggerDown = false;

      auto* const crosshairAsset =
         dynamic_cast<CrosshairAsset*>(m_sceneAssets[Asset::CROSSHAIR].get());

      assert(crosshairAsset);
      if (crosshairAsset)
      {
         crosshairAsset->Hide();
      }
   }

   if (!m_isRightTriggerDown
      && controllerState.rightTrigger > Constants::Xbox::TRIGGER_ACTUATION_THRESHOLD)
   {
      m_isRightTriggerDown = true;

      const auto ray = m_camera.ShootRayIntoScene(m_camera.GetViewport().center());
      m_controller.SelectNodeViaRay(m_camera, ray);
   }
   else if (m_isRightTriggerDown
      && controllerState.rightTrigger <= Constants::Xbox::TRIGGER_ACTUATION_THRESHOLD)
   {
      m_isRightTriggerDown = false;
   }
}

void GLCanvas::UpdateFPS()
{
   const auto now = std::chrono::system_clock::now();
   const auto millisecondsElapsed = std::max<unsigned int>(
      std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastFrameDrawTime).count(),
      1); ///< This will avoid division by zero.

   m_lastFrameDrawTime = now;

   constexpr auto movingAverageWindowSize{ 32 };
   if (m_frameRateDeque.size() > movingAverageWindowSize)
   {
      m_frameRateDeque.pop_front();
   }
   assert(m_frameRateDeque.size() <= movingAverageWindowSize);

   m_frameRateDeque.emplace_back(1000 / millisecondsElapsed);

   const int fpsSum = std::accumulate(std::begin(m_frameRateDeque), std::end(m_frameRateDeque), 0,
      [] (const int runningTotal, const int fps)
   {
      return runningTotal + fps;
   });

   assert(m_frameRateDeque.size() > 0);
   const auto averageFps = fpsSum / m_frameRateDeque.size();

   m_mainWindow.setWindowTitle(
      QString::fromStdString("D-Viz @ ")
      + QString::number(averageFps)
      + QString::fromStdString(" fps [*]"));
}

void GLCanvas::paintGL()
{
   if (m_isPaintingSuspended)
   {
      return;
   }

   m_graphicsDevice->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   if (m_mainWindow.ShouldShowFPS())
   {
      UpdateFPS();
   }

   assert(m_optionsManager);
   if (m_optionsManager->m_isLightAttachedToCamera)
   {
      assert(m_lights.size() > 0);
      m_lights.front().position = m_camera.GetPosition();
   }

   for (const auto& asset : m_sceneAssets)
   {
      assert(asset);
      asset->Render(m_camera, m_lights, *m_optionsManager);
   }
}
