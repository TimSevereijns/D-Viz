#include "glCanvas.h"

#include "../constants.h"

#include "Scene/debuggingRayAsset.h"
#include "Scene/gridAsset.h"
#include "Scene/nodeSelectionCrosshair.h"
#include "Scene/visualizationAsset.h"

#include "Visualizations/squarifiedTreemap.h"
#include "Utilities/scopeExit.hpp"

#include <QApplication>
#include <QMessageBox>

#include <iostream>
#include <sstream>
#include <utility>

namespace
{
   /**
    * @brief Helper function to set the specifed vertex and block count in the bottom status bar.
    *
    * @param[in] vertexCount        The readout value.
    * @param[in] mainWindow         The main window that contains the status bar.
    */
   void PrintMetadataToStatusBar(const unsigned int vertexCount, MainWindow& mainWindow)
   {
      std::wstringstream message;
      message.imbue(std::locale(""));
      message << std::fixed << vertexCount << L" vertices in "
         << (vertexCount / Block::VERTICES_PER_BLOCK) << L" blocks";

      mainWindow.SetStatusBarMessage(message.str());
   }

   /**
    * @brief Converts the given size of the file from bytes to the most human readable units.
    *
    * @param[in] sizeInBytes        The size (in bytes) to be converted to a more appropriate unit.
    *
    * @returns A std::pair encapsulating the converted file size, and corresponding unit readout
    * string.
    */
   auto GetFileSizeInMostAppropriateUnits(double sizeInBytes)
   {
      if (sizeInBytes < Constants::FileSize::oneKibibyte)
      {
         return std::make_pair<double, std::wstring>(std::move(sizeInBytes), L" bytes");
      }

      if (sizeInBytes < Constants::FileSize::oneMebibyte)
      {
         return std::make_pair<double, std::wstring>(
            sizeInBytes / Constants::FileSize::oneKibibyte, L" KiB");
      }

      if (sizeInBytes < Constants::FileSize::oneGibibyte)
      {
         return std::make_pair<double, std::wstring>(
            sizeInBytes / Constants::FileSize::oneMebibyte, L" MiB");
      }

      if (sizeInBytes < Constants::FileSize::oneTebibyte)
      {
         return std::make_pair<double, std::wstring>(
            sizeInBytes / Constants::FileSize::oneGibibyte, L" GiB");
      }

      return std::make_pair<double, std::wstring>(
         sizeInBytes / Constants::FileSize::oneTebibyte, L" TiB");
   }

   /**
    * @brief Computes the absolute file path of the selected node by traveling up tree.
    *
    * @param[in] node               The selected node.
    *
    * @returns The absolute file path.
    */
   std::wstring GetFullNodePath(const TreeNode<VizNode>& node)
   {
      std::vector<std::wstring> reversePath;
      reversePath.reserve(Tree<VizNode>::Depth(node));
      reversePath.emplace_back(node->file.name);

      const auto* currentNode = &node;

      while (currentNode->GetParent())
      {
         currentNode = currentNode->GetParent();
         reversePath.emplace_back(currentNode->GetData().file.name);
      }

      const auto completePath = std::accumulate(std::rbegin(reversePath), std::rend(reversePath),
         std::wstring{ }, [] (const std::wstring& path, const std::wstring& file)
      {
         return path + (!path.empty() ? L"/" : L"") + file;
      });

      assert(completePath.size() > 0);
      return completePath;
   }

   /**
    * @brief Clear sthe vertex and color data buffers from the specified asset, and will then reload
    * the now empty asset.
    *
    * @param[in] highlightAsset     The asset to be nuked and reloaded.
    * @param[in] camera             The camera used to view the asset in the scene.
    */
   void ClearAssetBuffersAndReload(SceneAsset& asset, const Camera& camera)
   {
      asset.SetVertexData(QVector<QVector3D>{ });
      asset.SetColorData(QVector<QVector3D>{ });
      asset.Reload(camera);
   }

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

GLCanvas::GLCanvas(QWidget* parent) :
   QOpenGLWidget{ parent },
   m_mainWindow{ reinterpret_cast<MainWindow*>(parent) }
{
   if (!m_mainWindow)
   {
      throw std::invalid_argument{ "Parent couldn't be interpreted as a MainWindow instance." };
   }

   m_settings = m_mainWindow->GetOptionsManager();

   m_camera.SetPosition(QVector3D{ 500, 100, 0 });

   setFocusPolicy(Qt::StrongFocus);

   QSurfaceFormat format;
   format.setDepthBufferSize(24);
   format.setSamples(8);
   format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
   setFormat(format);

   m_frameRedrawTimer.reset(new QTimer{ this });
   connect(m_frameRedrawTimer.get(), SIGNAL(timeout()), this, SLOT(update()));
   m_frameRedrawTimer->start(Constants::DESIRED_TIME_BETWEEN_FRAMES);

   m_cameraPositionTimer.reset(new QTimer{ this });
   connect(m_cameraPositionTimer.get(), SIGNAL(timeout()), this, SLOT(HandleInput()));
   m_cameraPositionTimer->start(Constants::DESIRED_TIME_BETWEEN_FRAMES);
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
   m_sceneAssets.emplace_back(std::make_unique<NodeSelectionCrosshair>(*m_graphicsDevice));

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
   m_camera.SetViewport(QRect{ QPoint{ 0, 0 }, QPoint{ width, height } });
}


void GLCanvas::CreateNewVisualization(VisualizationParameters& parameters)
{
   if (parameters.rootDirectory.empty())
   {
      return;
   }

   if (!m_theVisualization || parameters.forceNewScan)
   {
      m_theVisualization.reset(new SquarifiedTreeMap{ parameters });
      ScanDrive(parameters);
   }
}

void GLCanvas::ScanDrive(VisualizationParameters& vizParameters)
{
   const auto progressHandler =
      [&] (const std::uintmax_t numberOfFilesScanned)
   {
      std::wstringstream message;
      message.imbue(std::locale{ "" });
      message << std::fixed << L"Files Scanned: " << numberOfFilesScanned;
      m_mainWindow->SetStatusBarMessage(message.str());
   };

   const auto completionHandler =
      [&, vizParameters] (const std::uintmax_t numberOfFilesScanned,
      std::shared_ptr<Tree<VizNode>> fileTree) mutable
   {
      setCursor(Qt::WaitCursor);
      QApplication::processEvents();
      ON_SCOPE_EXIT{ setCursor(Qt::ArrowCursor); };

      std::wstringstream message;
      message.imbue(std::locale{ "" });
      message << std::fixed << L"Total Files Scanned: " << numberOfFilesScanned;
      m_mainWindow->SetStatusBarMessage(message.str());

      AskUserToLimitFileSize(numberOfFilesScanned, vizParameters);

      m_theVisualization->Parse(fileTree);
      m_theVisualization->UpdateBoundingBoxes();

      ReloadVisualization(vizParameters);
   };

   const DriveScanningParameters scanningParameters
   {
      vizParameters.rootDirectory,
      progressHandler,
      completionHandler
   };

   m_scanner.StartScanning(scanningParameters);
}

void GLCanvas::AskUserToLimitFileSize(
   const std::uintmax_t numberOfFilesScanned,
   VisualizationParameters& parameters) const
{
   assert(numberOfFilesScanned > 0);
   if (numberOfFilesScanned < 250'000)
   {
      return;
   }

   if (parameters.minimumFileSize < Constants::FileSize::oneMebibyte)
   {
      QMessageBox messageBox;
      messageBox.setIcon(QMessageBox::Warning);
      messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

      messageBox.setText(
         "More than a quarter million files were scanned. "                            \
         "Would you like to limit the visualized files to those 1 MiB or larger in "   \
         "order to reduce the load on the GPU and system memory?");

      const int election = messageBox.exec();
      switch (election)
      {
         case QMessageBox::Yes:
            parameters.minimumFileSize = Constants::FileSize::oneMebibyte;
            m_mainWindow->SetFilePruningComboBoxValue(Constants::FileSize::oneMebibyte);
            return;
         case QMessageBox::No:
            return;
         default:
            assert(false);
      }
   }
}

void GLCanvas::ReloadVisualization(const VisualizationParameters& parameters)
{
   const bool previousSuspensionState = m_isPaintingSuspended;
   m_isPaintingSuspended = true;
   ON_SCOPE_EXIT{ m_isPaintingSuspended = previousSuspensionState; };

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

   PrintMetadataToStatusBar(m_sceneAssets[Asset::TREEMAP]->GetVertexCount(), *m_mainWindow);
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

void GLCanvas::HandleNodeSelection(const TreeNode<VizNode>* selectedNode)
{
   const auto fileSize = selectedNode->GetData().file.size;
   assert(fileSize > 0);

   std::wstringstream message;
   message.imbue(std::locale{ "" });
   message.precision(2);

   const auto sizeAndUnits = GetFileSizeInMostAppropriateUnits(fileSize);
   message
      << GetFullNodePath(*selectedNode)
      << L"  |  "
      << std::fixed
      << sizeAndUnits.first
      << sizeAndUnits.second;

   assert(message.str().size() > 0);
   m_mainWindow->SetStatusBarMessage(message.str());

   if (m_selectedNode)
   {
      m_sceneAssets[Asset::TREEMAP]->UpdateVBO(
         *m_selectedNode,
         SceneAsset::UpdateAction::DESELECT,
         m_visualizationParameters);
   }

   m_sceneAssets[Asset::TREEMAP]->UpdateVBO(
      *selectedNode,
      SceneAsset::UpdateAction::SELECT,
      m_visualizationParameters);

   m_selectedNode = selectedNode;
}

void GLCanvas::HandleRightClick(const QPoint& point)
{
   if (!m_isVisualizationLoaded)
   {
      return;
   }

   const auto ray = m_camera.ShootRayIntoScene(point);
   const auto* selection = m_theVisualization->FindNearestIntersection(m_camera, ray,
      m_visualizationParameters);

   if (selection)
   {
      HandleNodeSelection(selection);
   }
   else
   {
      m_sceneAssets[Asset::TREEMAP]->UpdateVBO(
         *m_selectedNode,
         SceneAsset::UpdateAction::DESELECT,
         m_visualizationParameters);

       PrintMetadataToStatusBar(m_sceneAssets[Asset::TREEMAP]->GetVertexCount(), *m_mainWindow);
   }
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
      HandleRightClick(event->pos());
   }

   event->accept();
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

   if (event->buttons() & Qt::LeftButton)
   {
      m_camera.OffsetOrientation(
         m_settings->m_mouseSensitivity * deltaY,
         m_settings->m_mouseSensitivity * deltaX);
   }

   m_lastMousePosition = event->pos();

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
      if (delta > 0 && m_settings->m_cameraMovementSpeed < 1.0)
      {
         m_settings->m_cameraMovementSpeed += 0.01;
      }
      else if (delta < 0 && m_settings->m_cameraMovementSpeed > 0.01)
      {
         m_settings->m_cameraMovementSpeed -= 0.01;
      }

      m_mainWindow->SetCameraSpeedSpinner(static_cast<double>(m_settings->m_cameraMovementSpeed));
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

      m_mainWindow->SetFieldOfViewSlider(static_cast<float>(m_camera.GetFieldOfView()));
   }
}

void GLCanvas::HandleInput()
{
   assert(m_settings && m_mainWindow);

   const auto now = std::chrono::system_clock::now();
   ON_SCOPE_EXIT{ m_lastCameraPositionUpdatelTime = now; };

   if (m_settings->m_useXBoxController && m_mainWindow->IsXboxControllerConnected())
   {
      HandleXBoxControllerInput();

       return;
   }

   const auto millisecondsElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      now - m_lastCameraPositionUpdatelTime);

   const bool isKeyWDown = m_keyboardManager.IsKeyDown(Qt::Key_W);
   const bool isKeyADown = m_keyboardManager.IsKeyDown(Qt::Key_A);
   const bool isKeySDown = m_keyboardManager.IsKeyDown(Qt::Key_S);
   const bool isKeyDDown = m_keyboardManager.IsKeyDown(Qt::Key_D);

   if ((isKeyWDown && isKeySDown) || (isKeyADown && isKeyDDown))
   {
      return;
   }

   const auto cameraSpeed = m_settings->m_cameraMovementSpeed;

   if (isKeyWDown)
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * cameraSpeed * m_camera.Forward());
   }

   if (isKeyADown)
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * cameraSpeed * m_camera.Left());
   }

   if (isKeySDown)
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * cameraSpeed * m_camera.Backward());
   }

   if (isKeyDDown)
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * cameraSpeed * m_camera.Right());
   }
}

void GLCanvas::HandleXBoxControllerInput()
{
   const auto millisecondsElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now() - m_lastCameraPositionUpdatelTime);

   const XboxController::State& controllerState = m_mainWindow->GetXboxControllerState();
   const XboxController& controller = m_mainWindow->GetXboxControllerManager();

   const auto cameraSpeed = m_settings->m_cameraMovementSpeed / Constants::MOVEMENT_AMPLIFICATION;

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
      m_camera.OffsetPosition(millisecondsElapsed.count() * cameraSpeed * 0.4 * m_camera.Down());
   }

   if (controller.IsButtonDown(XINPUT_GAMEPAD_RIGHT_SHOULDER))
   {
      m_camera.OffsetPosition(millisecondsElapsed.count() * cameraSpeed * 0.4 * m_camera.Up());
   }

   HandleXboxThumbstickInput(controllerState);
   HandleXboxTriggerInput(controllerState);
}

void GLCanvas::HandleXboxThumbstickInput(const XboxController::State& controllerState)
{
   if (controllerState.rightThumbX || controllerState.rightThumbY)
   {
      const auto pitch =
         Constants::MOVEMENT_AMPLIFICATION *
         m_settings->m_mouseSensitivity *
         -controllerState.rightThumbY;

      const auto yaw =
         Constants::MOVEMENT_AMPLIFICATION *
         m_settings->m_mouseSensitivity *
         controllerState.rightThumbX;

      m_camera.OffsetOrientation(pitch, yaw);
   }

   if (controllerState.leftThumbY)
   {
      m_camera.OffsetPosition(
         Constants::MOVEMENT_AMPLIFICATION *
         m_settings->m_cameraMovementSpeed *
         controllerState.leftThumbY *
         m_camera.Forward());
   }

   if (controllerState.leftThumbX)
   {
      m_camera.OffsetPosition(
         Constants::MOVEMENT_AMPLIFICATION *
         m_settings->m_cameraMovementSpeed *
         controllerState.leftThumbX *
         m_camera.Right());
   }
}

void GLCanvas::HandleXboxTriggerInput(const XboxController::State& controllerState)
{
   if (controllerState.leftTrigger > Constants::XBOX_TRIGGER_ACTUATION_THRESHOLD &&
      !m_isLeftTriggerDown)
   {
      m_isLeftTriggerDown = true;

      auto* crosshairAsset =
         dynamic_cast<NodeSelectionCrosshair*>(m_sceneAssets[Asset::CROSSHAIR].get());

      assert(crosshairAsset);
      if (crosshairAsset)
      {
         crosshairAsset->ShowCrosshair(m_camera);
         crosshairAsset->Reload(m_camera);
      }
   }
   else if (controllerState.leftTrigger <= Constants::XBOX_TRIGGER_ACTUATION_THRESHOLD &&
      m_isLeftTriggerDown)
   {
      m_isLeftTriggerDown = false;

      auto* crosshairAsset =
         dynamic_cast<NodeSelectionCrosshair*>(m_sceneAssets[Asset::CROSSHAIR].get());

      assert(crosshairAsset);
      if (crosshairAsset)
      {
         crosshairAsset->HideCrosshair();
         crosshairAsset->Reload(m_camera);
      }
   }

   if (controllerState.rightTrigger > Constants::XBOX_TRIGGER_ACTUATION_THRESHOLD &&
      !m_isRightTriggerDown)
   {
      m_isRightTriggerDown = true;

      HandleRightClick(m_camera.GetViewport().center());
   }
   else if (controllerState.rightTrigger <= Constants::XBOX_TRIGGER_ACTUATION_THRESHOLD &&
      m_isRightTriggerDown)
   {
      m_isRightTriggerDown = false;
   }
}

void GLCanvas::UpdateFPS()
{
   const auto now = std::chrono::system_clock::now();
   const auto millisecondsElapsed = std::max<unsigned int>(
      std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastFrameDrawTime).count(),
      1); // This will avoid division by zero.

   m_lastFrameDrawTime = now;

   if (m_frameRateDeque.size() > 32)
   {
      m_frameRateDeque.pop_front();
   }
   assert(m_frameRateDeque.size() <= 32);

   m_frameRateDeque.emplace_back(1000 / millisecondsElapsed);

   const int fpsSum = std::accumulate(std::begin(m_frameRateDeque), std::end(m_frameRateDeque), 0,
      [] (const int runningTotal, const int fps)
   {
      return runningTotal + fps;
   });

   assert(m_frameRateDeque.size() > 0);
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

   m_graphicsDevice->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   UpdateFPS();

   assert(m_settings);
   if (m_settings->m_isLightAttachedToCamera)
   {
      assert(m_lights.size() > 0);
      m_lights.front().position = m_camera.GetPosition();
   }

   for (const auto& asset : m_sceneAssets)
   {
      assert(asset);
      asset->Render(m_camera, m_lights, *m_settings);
   }
}
