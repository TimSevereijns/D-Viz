#include "glCanvas.h"

#include "../constants.h"
#include "../ThirdParty/stopwatch.hpp"

#include "Scene/crosshairAsset.h"
#include "Scene/debuggingRayAsset.h"
#include "Scene/gridAsset.h"
#include "Scene/lightMarkerAsset.h"
#include "Scene/texturePreviewAsset.h"
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
      CROSSHAIR,     ///< CrosshairAsset
      LIGHT_MARKERS  ///< LightMarkerAsset
   };

   /**
    * @brief Computes and sets the vertex and color data for the light markers.
    *
    * @param[in] lights                   The lights in the scene to be marked.
    * @param[in, out] lightMarkerAsset    The scene asset to be updated
    */
   void InitializeLightMarkers(
      const std::vector<Light>& lights,
      LightMarkerAsset& lightMarkerAsset)
   {
      constexpr auto verticesPerMarker{ 6 };

      QVector<QVector3D> vertices;
      for (const auto& light : lights)
      {
         vertices
            << light.position + QVector3D{ 5.0f, 0.0f, 0.0f }
            << light.position - QVector3D{ 5.0f, 0.0f, 0.0f }
            << light.position + QVector3D{ 0.0f, 5.0f, 0.0f }
            << light.position - QVector3D{ 0.0f, 5.0f, 0.0f }
            << light.position + QVector3D{ 0.0f, 0.0f, 5.0f }
            << light.position - QVector3D{ 0.0f, 0.0f, 5.0f };
      }

      QVector<QVector3D> colors;
      for (std::size_t index{ 0 }; index < lights.size() * verticesPerMarker; ++index)
      {
         colors << Constants::Colors::WHITE;
      }

      lightMarkerAsset.SetVertexData(std::move(vertices));
      lightMarkerAsset.SetColorData(std::move(colors));
   }
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
   format.setDepthBufferSize(32);
   format.setSamples(8);
   format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
   setFormat(format);

   m_frameRedrawTimer = std::make_unique<QTimer>(this);
   connect(m_frameRedrawTimer.get(), SIGNAL(timeout()), this, SLOT(update()));
   m_frameRedrawTimer->start(Constants::Graphics::DESIRED_TIME_BETWEEN_FRAMES);

   m_inputCaptureTimer = std::make_unique<QTimer>(this);
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

   m_graphicsDevice->glCullFace(GL_BACK);

   m_sceneAssets.emplace_back(std::make_unique<GridAsset>(*m_graphicsDevice));
   m_sceneAssets.emplace_back(std::make_unique<VisualizationAsset>(*m_graphicsDevice));
   m_sceneAssets.emplace_back(std::make_unique<CrosshairAsset>(*m_graphicsDevice));
   m_sceneAssets.emplace_back(std::make_unique<LightMarkerAsset>(*m_graphicsDevice));
   //m_sceneAssets.emplace_back(std::make_unique<TexturePreviewAsset>(*m_graphicsDevice));

   auto* const lightMarkers = dynamic_cast<LightMarkerAsset*>(m_sceneAssets[LIGHT_MARKERS].get());
   assert(lightMarkers);

   InitializeLightMarkers(m_lights, *lightMarkers);

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
         SelectNodeViaRay(event->pos());
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

void GLCanvas::HighlightNodes(std::vector<const TreeNode<VizNode>*>& nodes)
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

   const auto deselectionCallback = [&] (auto& nodes) { RestoreHighlightedNodes(nodes); };
   const auto selectionCallback = [&] (auto& nodes) { HighlightNodes(nodes); };

   CanvasContextMenu menu{ m_keyboardManager };

   menu.addAction("Highlight Ancestors", [&]
   {
      constexpr auto clearSelected{ true };
      m_controller.ClearHighlightedNodes(deselectionCallback, clearSelected);
      m_controller.HighlightAncestors(*selectedNode, selectionCallback);
   });

   menu.addAction("Highlight Descendants", [&]
   {
      constexpr auto clearSelected{ true };
      m_controller.ClearHighlightedNodes(deselectionCallback, clearSelected);
      m_controller.HighlightDescendants(*selectedNode, selectionCallback);
   });

   if (selectedNode->GetData().file.type == FileType::REGULAR)
   {
      const auto entryText =
         QString::fromStdWString(L"Highlight All ")
         + QString::fromStdWString(selectedNode->GetData().file.extension)
         + QString::fromStdWString(L" Files");

      menu.addAction(entryText, [&]
      {
         constexpr auto clearSelected{ true };
         m_controller.ClearHighlightedNodes(deselectionCallback, clearSelected);
         m_controller.HighlightAllMatchingExtensions(*selectedNode, selectionCallback);
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
      now - m_lastCameraPositionUpdatelTime).count();

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
      m_camera.OffsetPosition(millisecondsElapsed * cameraSpeed * m_camera.Forward());
   }

   if (isAKeyDown)
   {
      m_camera.OffsetPosition(millisecondsElapsed * cameraSpeed * m_camera.Left());
   }

   if (isSKeyDown)
   {
      m_camera.OffsetPosition(millisecondsElapsed * cameraSpeed * m_camera.Backward());
   }

   if (isDKeyDown)
   {
      m_camera.OffsetPosition(millisecondsElapsed * cameraSpeed * m_camera.Right());
   }

//   const auto row1 = m_camera.GetProjectionViewMatrix().row(0);
//   const auto row2 = m_camera.GetProjectionViewMatrix().row(1);
//   const auto row3 = m_camera.GetProjectionViewMatrix().row(4);
//   const auto row4 = m_camera.GetProjectionViewMatrix().row(3);

//   std::cout << row1.x() << ", " << row1.y() << ", " << row1.z() << ", " << row1.w() << "\n";
//   std::cout << row2.x() << ", " << row2.y() << ", " << row2.z() << ", " << row2.w() << "\n";
//   std::cout << row3.x() << ", " << row3.y() << ", " << row3.z() << ", " << row3.w() << "\n";
//   std::cout << row4.x() << ", " << row4.y() << ", " << row4.z() << ", " << row4.w() << std::endl;
}

void GLCanvas::HandleXBoxControllerInput()
{
   const auto millisecondsElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now() - m_lastCameraPositionUpdatelTime).count();

   const XboxController::State& controllerState = m_mainWindow.GetXboxControllerState();
   const XboxController& controller = m_mainWindow.GetXboxControllerManager();

   const auto cameraSpeed =
       m_optionsManager->m_cameraMovementSpeed / Constants::Xbox::MOVEMENT_AMPLIFICATION;

   if (controller.IsButtonDown(XINPUT_GAMEPAD_DPAD_UP))
   {
      m_camera.OffsetPosition(millisecondsElapsed * cameraSpeed * m_camera.Forward());
   }

   if (controller.IsButtonDown(XINPUT_GAMEPAD_DPAD_LEFT))
   {
      m_camera.OffsetPosition(millisecondsElapsed * cameraSpeed * m_camera.Left());
   }

   if (controller.IsButtonDown(XINPUT_GAMEPAD_DPAD_DOWN))
   {
      m_camera.OffsetPosition(millisecondsElapsed * cameraSpeed * m_camera.Backward());
   }

   if (controller.IsButtonDown(XINPUT_GAMEPAD_DPAD_RIGHT))
   {
      m_camera.OffsetPosition(millisecondsElapsed * cameraSpeed * m_camera.Right());
   }

   if (controller.IsButtonDown(XINPUT_GAMEPAD_LEFT_SHOULDER))
   {
      m_camera.OffsetPosition(millisecondsElapsed * cameraSpeed * m_camera.Down());
   }

   if (controller.IsButtonDown(XINPUT_GAMEPAD_RIGHT_SHOULDER))
   {
      m_camera.OffsetPosition(millisecondsElapsed * cameraSpeed * m_camera.Up());
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
   const bool isLeftTriggerThresholdExceeded =
      controllerState.leftTrigger > Constants::Xbox::TRIGGER_ACTUATION_THRESHOLD;

   if (!m_isLeftTriggerDown && isLeftTriggerThresholdExceeded)
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
   else if (m_isLeftTriggerDown && !isLeftTriggerThresholdExceeded)
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

   const bool isRightTriggerThresholdExceeded =
      controllerState.rightTrigger > Constants::Xbox::TRIGGER_ACTUATION_THRESHOLD;

   if (!m_isRightTriggerDown && isRightTriggerThresholdExceeded)
   {
      m_isRightTriggerDown = true;

      SelectNodeViaRay(m_camera.GetViewport().center());
   }
   else if (m_isRightTriggerDown && !isRightTriggerThresholdExceeded)
   {
      m_isRightTriggerDown = false;
   }
}

void GLCanvas::SelectNodeViaRay(const QPoint& rayOrigin)
{
   const auto deselectionCallback = [&] (std::vector<const TreeNode<VizNode>*>& nodes)
   {
      RestoreHighlightedNodes(nodes);
      RestoreSelectedNode();
   };

   const auto selectionCallback = [&] (auto* node) { SelectNode(node); };

   const auto ray = m_camera.ShootRayIntoScene(rayOrigin);

   m_controller.SelectNodeViaRay(m_camera, ray, deselectionCallback, selectionCallback );
}

void GLCanvas::UpdateFrameTime(const std::chrono::microseconds& elapsedTime)
{
   constexpr auto movingAverageWindowSize{ 64 };
   if (m_frameTimeDeque.size() > movingAverageWindowSize)
   {
      m_frameTimeDeque.pop_front();
   }
   assert(m_frameTimeDeque.size() <= movingAverageWindowSize);

   m_frameTimeDeque.emplace_back(static_cast<int>(elapsedTime.count()));

   const int total = std::accumulate(std::begin(m_frameTimeDeque), std::end(m_frameTimeDeque), 0,
      [] (const int runningTotal, const int frameTime) noexcept
   {
      return runningTotal + frameTime;
   });

   assert(m_frameTimeDeque.size() > 0);
   const auto averageFrameTime = total / m_frameTimeDeque.size();

   m_mainWindow.setWindowTitle(
      QString::fromStdWString(L"D-Viz @ ")
      + QString::number(averageFrameTime)
      + QString::fromStdWString(L" \xB5s / frame"));
}

void GLCanvas::paintGL()
{
   const auto elapsedTime = Stopwatch<std::chrono::microseconds>(
      [&] () noexcept
   {
      if (m_isPaintingSuspended)
      {
         return;
      }

      m_graphicsDevice->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
   }).GetElapsedTime();

   if (m_mainWindow.ShouldShowFrameTime())
   {
      UpdateFrameTime(elapsedTime);
   }
}
