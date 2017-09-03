#include "glCanvas.h"

#include "../constants.h"

#include "canvasContextMenu.h"
#include "Scene/crosshairAsset.h"
#include "Scene/debuggingRayAsset.h"
#include "Scene/gridAsset.h"
#include "Scene/lightMarkerAsset.h"
#include "Scene/treemapAsset.h"
#include "Stopwatch/Stopwatch.hpp"
#include "Utilities/operatingSystemSpecific.hpp"
#include "Utilities/scopeExit.hpp"
#include "Visualizations/squarifiedTreemap.h"

#include <QApplication>
#include <QMenu>
#include <QMessageBox>

#include <spdlog/spdlog.h>

#include <iostream>

namespace
{
   /**
    * @brief Provides an easier way to index into the asset vector, than memorizing indices.
    */
   inline namespace Asset
   {
      static constexpr const char GRID[] = "Grid";
      static constexpr const char TREEMAP[] = "TreeMap";
      static constexpr const char CROSSHAIR[] = "Crosshair";
      static constexpr const char LIGHT_MARKERS[] = "Light Markers";
   }

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

   connect(&m_frameRedrawTimer, &QTimer::timeout, this, &GLCanvas::RunMainLoop);
   m_frameRedrawTimer.start(Constants::Graphics::DESIRED_TIME_BETWEEN_FRAMES);
}

void GLCanvas::RunMainLoop()
{
  HandleUserInput();
  update();
}

void GLCanvas::initializeGL()
{
   m_graphicsDevice.initializeOpenGLFunctions();

   m_graphicsDevice.glEnable(GL_DEPTH_TEST);
   m_graphicsDevice.glEnable(GL_CULL_FACE);
   m_graphicsDevice.glEnable(GL_MULTISAMPLE);
   m_graphicsDevice.glEnable(GL_LINE_SMOOTH);

   m_sceneAssets.emplace_back(AssetNameAndPtr
   {
      Asset::GRID,
      std::make_unique<GridAsset>(m_graphicsDevice)
   });

   m_sceneAssets.emplace_back(AssetNameAndPtr
   {
      Asset::TREEMAP,
      std::make_unique<TreemapAsset>(m_graphicsDevice)
   });

   m_sceneAssets.emplace_back(AssetNameAndPtr
   {
      Asset::CROSSHAIR,
      std::make_unique<CrosshairAsset>(m_graphicsDevice)
   });

   m_sceneAssets.emplace_back(AssetNameAndPtr
   {
      Asset::LIGHT_MARKERS,
      std::make_unique<LightMarkerAsset>(m_graphicsDevice)
   });

   auto* lightMarkers = GetAsset<LightMarkerAsset>(Asset::LIGHT_MARKERS);
   InitializeLightMarkers(m_lights, *lightMarkers);

   for (const auto& asset : m_sceneAssets)
   {
      asset.pointer->LoadShaders();
      asset.pointer->Initialize();
   }
}

void GLCanvas::resizeGL(int width, int height)
{
   if (height == 0)
   {
      height = 1;
   }

   m_graphicsDevice.glViewport(0, 0, width, height);

   m_camera.SetViewport(QRect{ QPoint{ 0, 0 }, QPoint{ width, height } });
}

void GLCanvas::ReloadVisualization()
{
   const bool previousSuspensionState = m_isPaintingSuspended;
   ON_SCOPE_EXIT noexcept { m_isPaintingSuspended = previousSuspensionState; };

   m_isPaintingSuspended = true;

   auto* const treemap = GetAsset<TreemapAsset>(Asset::TREEMAP);
   const auto parameters = m_controller.GetVisualizationParameters();
   const auto blockCount = treemap->LoadBufferData(m_controller.GetTree(), parameters);

   for (const auto& asset : m_sceneAssets)
   {
      asset.pointer->Reload();
   }

   assert(blockCount == treemap->GetBlockCount());

   m_controller.PrintMetadataToStatusBar();
}

void GLCanvas::SetFieldOfView(float fieldOfView)
{
   m_camera.SetFieldOfView(fieldOfView);
}

void GLCanvas::keyPressEvent(QKeyEvent* const event)
{
   if (!event)
   {
      return;
   }

   if (event->isAutoRepeat())
   {
      event->ignore();
      return;
   }

   constexpr auto state = KeyboardManager::KEY_STATE::DOWN;
   m_keyboardManager.UpdateKeyState(static_cast<Qt::Key>(event->key()), state);

   event->accept();
}

void GLCanvas::keyReleaseEvent(QKeyEvent* const event)
{
   if (!event)
   {
      return;
   }

   if (event->isAutoRepeat())
   {
      event->ignore();
      return;
   }

   constexpr auto state = KeyboardManager::KEY_STATE::UP;
   m_keyboardManager.UpdateKeyState(static_cast<Qt::Key>(event->key()), state);

   event->accept();
}

void GLCanvas::mousePressEvent(QMouseEvent* const event)
{
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

      const auto newSpeed = static_cast<double>(m_optionsManager->m_cameraMovementSpeed);
      m_mainWindow.SetCameraSpeedSpinner(newSpeed);
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

void GLCanvas::SelectNode(const Tree<VizFile>::Node* const node)
{
   auto* const treemap = GetAsset<TreemapAsset>(Asset::TREEMAP);
   assert(treemap);

   treemap->UpdateVBO(
      *node,
      SceneAsset::UpdateAction::SELECT,
      m_controller.GetVisualizationParameters());
}

void GLCanvas::RestoreSelectedNode()
{
   if (!m_controller.GetSelectedNode())
   {
      return;
   }

   auto* const treemap = GetAsset<TreemapAsset>(Asset::TREEMAP);
   assert(treemap);

   treemap->UpdateVBO(
      *m_controller.GetSelectedNode(),
      SceneAsset::UpdateAction::DESELECT,
      m_controller.GetVisualizationParameters());
}

void GLCanvas::HighlightNodes(std::vector<const Tree<VizFile>::Node*>& nodes)
{
   for (const auto* const node : nodes)
   {
      SelectNode(node);
   }
}

void GLCanvas::RestoreHighlightedNodes(std::vector<const Tree<VizFile>::Node*>& nodes)
{
   auto* const treemap = GetAsset<TreemapAsset>(Asset::TREEMAP);
   assert(treemap);

   for (const auto* const node : nodes)
   {
      treemap->UpdateVBO(
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
      fmt::WMemoryWriter writer;
      writer << L"Highlight All " << selectedNode->GetData().file.extension << L" Files";

      menu.addAction(QString::fromStdWString(writer.c_str()), [&]
      {
         constexpr auto clearSelected{ true };
         m_controller.ClearHighlightedNodes(deselectionCallback, clearSelected);
         m_controller.HighlightAllMatchingExtensions(*selectedNode, selectionCallback);
      });
   }

   menu.addSeparator();

   menu.addAction("Isolate File Type", [&]
   {
      constexpr auto clearSelected{ true };
      m_controller.ClearHighlightedNodes(deselectionCallback, clearSelected);
      // @todo Relaad the visualization with updated parameters.
   });

   menu.addSeparator();

   menu.addAction("Show in Explorer", [&]
   {
      OperatingSystemSpecific::LaunchFileExplorer(*selectedNode);
   });

   const QPoint globalPoint = mapToGlobal(point);
   menu.exec(globalPoint);
}

void GLCanvas::HandleUserInput()
{
   assert(m_optionsManager);

   const auto now = std::chrono::system_clock::now();
   ON_SCOPE_EXIT noexcept { m_lastCameraPositionUpdatelTime = now; };

   const auto millisecondsElapsedSinceLastUpdate =
      std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastCameraPositionUpdatelTime);

   HandleGamepadInput(millisecondsElapsedSinceLastUpdate);
   HandleKeyboardInput(millisecondsElapsedSinceLastUpdate);
}

void GLCanvas::HandleKeyboardInput(const std::chrono::milliseconds& elapsedTime)
{
   const bool isWKeyDown = m_keyboardManager.IsKeyDown(Qt::Key_W);
   const bool isAKeyDown = m_keyboardManager.IsKeyDown(Qt::Key_A);
   const bool isSKeyDown = m_keyboardManager.IsKeyDown(Qt::Key_S);
   const bool isDKeyDown = m_keyboardManager.IsKeyDown(Qt::Key_D);

   if ((isWKeyDown && isSKeyDown) || (isAKeyDown && isDKeyDown))
   {
      return;
   }

   const auto millisecondsElapsed = elapsedTime.count();
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
}

void GLCanvas::HandleGamepadInput(const std::chrono::milliseconds& elapsedTime)
{
   if (!m_mainWindow.GetGamepad().isConnected())
   {
      return;
   }

   const auto& gamepad = m_mainWindow.GetGamepad();

   HandleGamepadKeyInput(gamepad, elapsedTime);
   HandleGamepadThumbstickInput(gamepad);
   HandleGamepadTriggerInput(gamepad);
}

void GLCanvas::HandleGamepadKeyInput(
   const Gamepad& gamepad,
   const std::chrono::milliseconds& elapsedTime)
{
   const auto millisecondsElapsed = elapsedTime.count();
   const auto cameraSpeed =
      m_optionsManager->m_cameraMovementSpeed / Constants::Input::MOVEMENT_AMPLIFICATION;

   if (gamepad.buttonUp())
   {
      m_camera.OffsetPosition(millisecondsElapsed * cameraSpeed * m_camera.Forward());
   }

   if (gamepad.buttonLeft())
   {
      m_camera.OffsetPosition(millisecondsElapsed * cameraSpeed * m_camera.Left());
   }

   if (gamepad.buttonDown())
   {
      m_camera.OffsetPosition(millisecondsElapsed * cameraSpeed * m_camera.Backward());
   }

   if (gamepad.buttonRight())
   {
      m_camera.OffsetPosition(millisecondsElapsed * cameraSpeed * m_camera.Right());
   }

   if (gamepad.buttonL1())
   {
      m_camera.OffsetPosition(millisecondsElapsed * cameraSpeed * m_camera.Down());
   }

   if (gamepad.buttonR1())
   {
      m_camera.OffsetPosition(millisecondsElapsed * cameraSpeed * m_camera.Up());
   }
}

void GLCanvas::HandleGamepadThumbstickInput(const Gamepad& gamepad)
{
   if (gamepad.axisRightX() || gamepad.axisRightY())
   {
      const auto pitch =
         Constants::Input::MOVEMENT_AMPLIFICATION
         * m_optionsManager->m_mouseSensitivity
         * gamepad.axisRightY();

      const auto yaw =
         Constants::Input::MOVEMENT_AMPLIFICATION
         * m_optionsManager->m_mouseSensitivity
         * gamepad.axisRightX();

      m_camera.OffsetOrientation(pitch, yaw);
   }

   if (gamepad.axisLeftY())
   {
      m_camera.OffsetPosition(
         Constants::Input::MOVEMENT_AMPLIFICATION
         * m_optionsManager->m_cameraMovementSpeed
         * -gamepad.axisLeftY()
         * m_camera.Forward());
   }

   if (gamepad.axisLeftX())
   {
      m_camera.OffsetPosition(
         Constants::Input::MOVEMENT_AMPLIFICATION
         * m_optionsManager->m_cameraMovementSpeed
         * gamepad.axisLeftX()
         * m_camera.Right());
   }
}

void GLCanvas::HandleGamepadTriggerInput(const Gamepad& gamepad)
{
   if (!m_isLeftTriggerDown && gamepad.IsLeftTriggerDown())
   {
      m_isLeftTriggerDown = true;

      auto* const crosshairAsset = GetAsset<CrosshairAsset>(Asset::CROSSHAIR);
      crosshairAsset->Show(m_camera);
   }
   else if (m_isLeftTriggerDown && !gamepad.IsLeftTriggerDown())
   {
       m_isLeftTriggerDown = false;

       auto* const crosshairAsset = GetAsset<CrosshairAsset>(Asset::CROSSHAIR);
       crosshairAsset->Hide();
   }

   if (!m_isRightTriggerDown && gamepad.IsRightTriggerDown())
   {
      m_isRightTriggerDown = true;

      SelectNodeViaRay(m_camera.GetViewport().center());
   }
   else if (m_isRightTriggerDown && !gamepad.IsRightTriggerDown())
   {
      m_isRightTriggerDown = false;
   }
}

void GLCanvas::SelectNodeViaRay(const QPoint& rayOrigin)
{
   const auto deselectionCallback = [&] (std::vector<const Tree<VizFile>::Node*>& nodes)
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
   if (m_isPaintingSuspended)
   {
      return;
   }

   const auto elapsedTime = Stopwatch<std::chrono::microseconds>(
      [&] () noexcept
   {
      m_graphicsDevice.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      assert(m_optionsManager);
      if (m_optionsManager->m_isLightAttachedToCamera)
      {
         assert(m_lights.size() > 0);
         m_lights.front().position = m_camera.GetPosition();
      }

      for (const auto& asset : m_sceneAssets)
      {
         assert(asset.pointer);
         asset.pointer->Render(m_camera, m_lights, *m_optionsManager);
      }
   }).GetElapsedTime();

   if (m_mainWindow.ShouldShowFrameTime())
   {
      UpdateFrameTime(elapsedTime);
   }
}
