#include "Viewport/glCanvas.h"

#include "Stopwatch/Stopwatch.hpp"
#include "Utilities/operatingSystemSpecific.hpp"
#include "Utilities/scopeExit.hpp"
#include "Viewport/gamepadContextMenu.h"
#include "Viewport/mouseContextMenu.h"
#include "Visualizations/squarifiedTreemap.h"
#include "Windows/mainWindow.h"
#include "constants.h"
#include "controller.h"

#include <QApplication>
#include <QMenu>
#include <QMessageBox>

#include <gsl/gsl_assert>

namespace
{
    /**
     * @brief Computes and sets the vertex and color data for the light markers.
     *
     * @param[in] lights                   The lights in the scene to be marked.
     * @param[out] lightMarkerAsset        The scene asset to be updated
     */
    void
    InitializeLightMarkers(const std::vector<Light>& lights, Assets::LightMarker& lightMarkerAsset)
    {
        constexpr auto verticesPerMarker{ 6 };

        QVector<QVector3D> vertices;
        for (const auto& light : lights) {
            vertices << light.position + QVector3D{ 5.0f, 0.0f, 0.0f }
                     << light.position - QVector3D{ 5.0f, 0.0f, 0.0f }
                     << light.position + QVector3D{ 0.0f, 5.0f, 0.0f }
                     << light.position - QVector3D{ 0.0f, 5.0f, 0.0f }
                     << light.position + QVector3D{ 0.0f, 0.0f, 5.0f }
                     << light.position - QVector3D{ 0.0f, 0.0f, 5.0f };
        }

        QVector<QVector3D> colors;
        for (std::size_t index{ 0 }; index < lights.size() * verticesPerMarker; ++index) {
            colors << Constants::Colors::White;
        }

        lightMarkerAsset.SetVertexCoordinates(std::move(vertices));
        lightMarkerAsset.SetVertexColors(std::move(colors));
    }

    /**
     * @brief Creates the "Highlight All..." message for the context menus.
     *
     * @param[in] node              The node that the menu is based off of.
     *
     * @returns The formatted label.
     */
    QString GetHighlightExtensionLabel(const Tree<VizBlock>::Node& node)
    {
        const auto extension = node.GetData().file.extension.empty()
                                   ? L"Extensionless"
                                   : L"\"" + node.GetData().file.extension + L"\"";

        const auto label = L"Highlight All " + extension + L" Files";
        return QString::fromStdWString(label);
    }
} // namespace

GLCanvas::GLCanvas(Controller& controller, QWidget* parent)
    : QOpenGLWidget{ parent },
      m_controller{ controller },
      m_mainWindow{ *(dynamic_cast<MainWindow*>(parent)) }
{
    m_camera.SetPosition(QVector3D{ 500, 100, 0 });
    m_camera.SetFarPlane(10'000.0f);

    setFocusPolicy(Qt::StrongFocus);

    QSurfaceFormat format;
    format.setDepthBufferSize(32);
    format.setSamples(8);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    setFormat(format);

    connect(&m_frameRedrawTimer, &QTimer::timeout, this, &GLCanvas::RunMainLoop);
    m_frameRedrawTimer.start(Constants::Graphics::DesiredTimeBetweenFrames);
}

void GLCanvas::RunMainLoop()
{
    HandleUserInput();
    update();
}

void GLCanvas::initializeGL()
{
    m_openGLContext.initializeOpenGLFunctions();

    m_openGLContext.glEnable(GL_DEPTH_TEST);
    m_openGLContext.glEnable(GL_CULL_FACE);
    m_openGLContext.glEnable(GL_MULTISAMPLE);
    m_openGLContext.glEnable(GL_LINE_SMOOTH);

    RegisterAsset<Assets::Tag::Grid>();
    RegisterAsset<Assets::Tag::OriginMarker>();
    RegisterAsset<Assets::Tag::Treemap>();
    RegisterAsset<Assets::Tag::Crosshair>();
    RegisterAsset<Assets::Tag::LightMarker>();
    RegisterAsset<Assets::Tag::Frustum>();

    auto* lightMarkers = GetAsset<Assets::Tag::LightMarker>();
    InitializeLightMarkers(m_lights, *lightMarkers);

    for (const auto& tagAndAsset : m_sceneAssets) {
        tagAndAsset.asset->LoadShaders();
        tagAndAsset.asset->Initialize();
    }
}

template <typename AssetTag> void GLCanvas::RegisterAsset()
{
    m_sceneAssets.emplace_back(TagAndAsset{
        std::make_unique<AssetTag>(),
        std::make_unique<typename AssetTag::AssetType>(m_controller, m_openGLContext) });
}

template <typename RequestedAsset>
typename RequestedAsset::AssetType* GLCanvas::GetAsset() const noexcept
{
    const auto itr = std::find_if(
        std::begin(m_sceneAssets), std::end(m_sceneAssets),
        [targetID = RequestedAsset{}.GetID()](const auto& tagAndAsset) noexcept {
            return tagAndAsset.tag->GetID() == targetID;
        });

    if (itr == std::end(m_sceneAssets)) {
        return nullptr;
    }

    return static_cast<typename RequestedAsset::AssetType*>(itr->asset.get());
}

template <typename TagType> void GLCanvas::ToggleAssetVisibility(bool shouldEnable) const noexcept
{
    auto* const asset = GetAsset<TagType>();

    if (shouldEnable) {
        asset->Show();
    } else {
        asset->Hide();
    }
}

template <>
void GLCanvas::ToggleAssetVisibility<Assets::Tag::Frustum>(bool shouldEnable) const noexcept
{
    auto* const asset = GetAsset<Assets::Tag::Frustum>();

    if (shouldEnable) {
        asset->GenerateFrusta(m_camera);
        asset->Show();
    } else {
        asset->Hide();
    }
}

void GLCanvas::resizeGL(int width, int height)
{
    if (height == 0) {
        height = 1;
    }

    m_openGLContext.glViewport(0, 0, width, height);
    m_camera.SetViewport(QRect{ QPoint{ 0, 0 }, QPoint{ width, height } });

    auto* const frusta = GetAsset<Assets::Tag::Frustum>();
    frusta->GenerateFrusta(m_camera);
}

void GLCanvas::ReloadVisualization()
{
    const auto previousSuspensionState = m_isPaintingSuspended;
    const ScopeExit onScopeExit = [&]() noexcept
    {
        m_isPaintingSuspended = previousSuspensionState;
    };

    m_isPaintingSuspended = true;

    auto* const treemap = GetAsset<Assets::Tag::Treemap>();
    const auto blockCount = treemap->LoadBufferData(m_controller.GetTree());

    Expects(blockCount == treemap->GetBlockCount());

    for (const auto& tagAndAsset : m_sceneAssets) {
        tagAndAsset.asset->Refresh();
    }

    m_controller.PrintMetadataToStatusBar();
}

void GLCanvas::ApplyColorScheme()
{
    const auto deselectionCallback = [&](auto& nodes) { RestoreHighlightedNodes(nodes); };

    m_controller.ClearHighlightedNodes(deselectionCallback);

    auto* const treemap = GetAsset<Assets::Tag::Treemap>();
    treemap->ReloadColorBufferData(m_controller.GetTree());
    treemap->Refresh();

    const auto* selectedNode = m_controller.GetSelectedNode();
    if (selectedNode) {
        SelectNode(*selectedNode);
    }
}

void GLCanvas::SetFieldOfView(int fieldOfView)
{
    m_camera.SetFieldOfView(fieldOfView);
}

void GLCanvas::keyPressEvent(QKeyEvent* const event)
{
    if (!event) {
        return;
    }

    if (event->isAutoRepeat()) {
        event->ignore();
        return;
    }

    constexpr auto state = KeyboardManager::KEY_STATE::DOWN;
    m_keyboardManager.UpdateKeyState(static_cast<Qt::Key>(event->key()), state);

    event->accept();
}

void GLCanvas::keyReleaseEvent(QKeyEvent* const event)
{
    if (!event) {
        return;
    }

    if (event->isAutoRepeat()) {
        event->ignore();
        return;
    }

    constexpr auto state = KeyboardManager::KEY_STATE::UP;
    m_keyboardManager.UpdateKeyState(static_cast<Qt::Key>(event->key()), state);

    event->accept();
}

void GLCanvas::mousePressEvent(QMouseEvent* const event)
{
    if (!event) {
        return;
    }

    m_lastMousePosition = event->pos();

    if (event->button() == Qt::RightButton) {
        if (m_keyboardManager.IsKeyDown(Qt::Key_Control)) {
            ShowContextMenu(m_lastMousePosition);
        } else {
            SelectNodeViaRay(event->pos());
        }
    } else if (event->button() == Qt::LeftButton) {
        if (!m_isLeftMouseButtonDown) {
            m_isLeftMouseButtonDown = true;
            m_startOfMouseLookEvent = std::chrono::system_clock::now();
        }
    }

    event->accept();
}

void GLCanvas::mouseReleaseEvent(QMouseEvent* const event)
{
    if (!event) {
        return;
    }

    if (event->button() == Qt::LeftButton) {
        m_isLeftMouseButtonDown = false;

        if (m_isCursorHidden) {
            const auto globalCursorPosition = mapToGlobal(m_camera.GetViewport().center());
            QCursor::setPos(globalCursorPosition.x(), globalCursorPosition.y());

            setCursor(Qt::ArrowCursor);
            m_isCursorHidden = false;
        }
    }
}

void GLCanvas::mouseMoveEvent(QMouseEvent* const event)
{
    if (!event) {
        return;
    }

    const auto deltaX = event->x() - m_lastMousePosition.x();
    const auto deltaY = event->y() - m_lastMousePosition.y();

    if (!m_isCursorHidden) {
        m_lastMousePosition = event->pos();
    }

    if (event->buttons() & Qt::LeftButton) {
        m_camera.OffsetOrientation(
            m_controller.GetSettingsManager().GetMouseSensitivity() * deltaY,
            m_controller.GetSettingsManager().GetMouseSensitivity() * deltaX);

#ifdef Q_OS_UNIX
        // @note This only appears to work as expected on Linux. The camera angle jumps when the
        // cursor is hidden on Windows; I think there might still be messages in the event queue
        // that messes up the delta computation. @todo Investigate...

        const auto timeSinceStartOfLookEvent = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now() - m_startOfMouseLookEvent);

        if (timeSinceStartOfLookEvent >= std::chrono::seconds{ 2 }) {
            if (!m_isCursorHidden) {
                m_isCursorHidden = true;
                setCursor(Qt::BlankCursor);
            }

            const auto cursorPositionOnCanvas = m_camera.GetViewport().center();
            const auto cursorPositionOnMonitor = mapToGlobal(cursorPositionOnCanvas);
            QCursor::setPos(cursorPositionOnMonitor);

            m_lastMousePosition = cursorPositionOnCanvas;
        }
#endif
    }

    event->accept();
}

void GLCanvas::wheelEvent(QWheelEvent* const event)
{
    if (!event) {
        return;
    }

    event->accept();

    if (event->orientation() != Qt::Vertical) {
        return;
    }

    const auto cameraSpeed = m_controller.GetSettingsManager().GetCameraSpeed();
    const auto delta = event->delta();

    if (m_keyboardManager.IsKeyUp(Qt::Key_Shift)) {
        if (delta > 0 && cameraSpeed < 1.0) {
            m_mainWindow.SetCameraSpeedSpinner(cameraSpeed + 0.01);
        } else if (delta < 0 && cameraSpeed > 0.01) {
            m_mainWindow.SetCameraSpeedSpinner(cameraSpeed - 0.01);
        }
    } else {
        if (delta < 0) {
            m_camera.IncreaseFieldOfView();
        } else if (delta > 0) {
            m_camera.DecreaseFieldOfView();
        }

        m_mainWindow.SetFieldOfViewSlider(m_camera.GetVerticalFieldOfView());
    }
}

void GLCanvas::SelectNode(const Tree<VizBlock>::Node& node)
{
    auto* const treemap = GetAsset<Assets::Tag::Treemap>();
    treemap->SetNodeColor(node, Constants::Colors::CanaryYellow);
}

void GLCanvas::RestoreSelectedNode(const Tree<VizBlock>::Node& node)
{
    auto* const treemap = GetAsset<Assets::Tag::Treemap>();

    const auto restorationColor = m_controller.DetermineNodeColor(node);
    treemap->SetNodeColor(node, restorationColor);
}

void GLCanvas::HighlightNodes(std::vector<const Tree<VizBlock>::Node*>& nodes)
{
    auto* const treemap = GetAsset<Assets::Tag::Treemap>();

    for (const auto* const node : nodes) {
        treemap->SetNodeColor(*node, Constants::Colors::SlateGray);
    }
}

void GLCanvas::RestoreHighlightedNodes(std::vector<const Tree<VizBlock>::Node*>& nodes)
{
    auto* const treemap = GetAsset<Assets::Tag::Treemap>();

    for (const auto* const node : nodes) {
        const auto restorationColor = m_controller.DetermineNodeColor(*node);
        treemap->SetNodeColor(*node, restorationColor);
    }
}

void GLCanvas::ShowGamepadContextMenu()
{
    const auto thereExistHighlightedNodes = !m_controller.GetHighlightedNodes().empty();
    const auto* const selectedNode = m_controller.GetSelectedNode();

    if (!thereExistHighlightedNodes && !selectedNode) {
        return;
    }

    const auto unhighlightCallback = [&](auto& nodes) { RestoreHighlightedNodes(nodes); };
    const auto highlightCallback = [&](auto& nodes) { HighlightNodes(nodes); };
    const auto selectionCallback = [&](auto& node) { SelectNode(node); };

    // @note Qt will manage the lifetime of this object:
    m_gamepadContextMenu = new GamepadContextMenu{ m_mainWindow.GetGamepad(), this };

    if (thereExistHighlightedNodes) {
        m_gamepadContextMenu->AddEntry(
            "Clear Highlights", [=] { m_controller.ClearHighlightedNodes(unhighlightCallback); });
    }

    if (selectedNode) {
        m_gamepadContextMenu->AddEntry("Highlight Ancestors", [=] {
            m_controller.ClearHighlightedNodes(unhighlightCallback);
            m_controller.HighlightAncestors(*selectedNode, highlightCallback);
        });

        m_gamepadContextMenu->AddEntry("Highlight Descendants", [=] {
            m_controller.ClearHighlightedNodes(unhighlightCallback);
            m_controller.HighlightDescendants(*selectedNode, highlightCallback);
        });

        if (selectedNode->GetData().file.type == FileType::REGULAR) {
            const auto message = GetHighlightExtensionLabel(*selectedNode);

            m_gamepadContextMenu->AddEntry(message, [=] {
                m_controller.ClearHighlightedNodes(unhighlightCallback);
                m_controller.HighlightAllMatchingExtensions(*selectedNode, highlightCallback);
                m_controller.SelectNode(*selectedNode, selectionCallback);
            });
        }

        m_gamepadContextMenu->AddEntry(
            "Show in Explorer", [=] { OS::LaunchFileExplorer(*selectedNode); });
    }

    m_gamepadContextMenu->move(mapToGlobal(QPoint{ 0, 0 }));
    m_gamepadContextMenu->resize(width(), height());

    m_gamepadContextMenu->ComputeLayout();

    m_gamepadContextMenu->show();
    m_gamepadContextMenu->raise();
}

void GLCanvas::ShowContextMenu(const QPoint& point)
{
    const auto thereExistHighlightedNodes = !m_controller.GetHighlightedNodes().empty();
    const auto* const selectedNode = m_controller.GetSelectedNode();

    if (!thereExistHighlightedNodes && !selectedNode) {
        return;
    }

    const auto unhighlightCallback = [&](auto& nodes) { RestoreHighlightedNodes(nodes); };
    const auto highlightCallback = [&](auto& nodes) { HighlightNodes(nodes); };
    const auto selectionCallback = [&](auto& node) { SelectNode(node); };

    MouseContextMenu menu{ m_keyboardManager };

    if (thereExistHighlightedNodes) {
        menu.addAction(
            "Clear Highlights", [&] { m_controller.ClearHighlightedNodes(unhighlightCallback); });

        menu.addSeparator();
    }

    if (selectedNode) {
        menu.addAction("Highlight Ancestors", [&] {
            m_controller.ClearHighlightedNodes(unhighlightCallback);
            m_controller.HighlightAncestors(*selectedNode, highlightCallback);
        });

        menu.addAction("Highlight Descendants", [&] {
            m_controller.ClearHighlightedNodes(unhighlightCallback);
            m_controller.HighlightDescendants(*selectedNode, highlightCallback);
        });

        if (selectedNode->GetData().file.type == FileType::REGULAR) {
            const auto message = GetHighlightExtensionLabel(*selectedNode);

            menu.addAction(message, [&] {
                m_controller.ClearHighlightedNodes(unhighlightCallback);
                m_controller.HighlightAllMatchingExtensions(*selectedNode, highlightCallback);
                m_controller.SelectNode(*selectedNode, selectionCallback);
            });
        }

        menu.addSeparator();

        menu.addAction("Show in Explorer", [&] { OS::LaunchFileExplorer(*selectedNode); });
    }

    const QPoint globalPoint = mapToGlobal(point);
    menu.exec(globalPoint);
}

void GLCanvas::HandleUserInput()
{
    const auto now = std::chrono::system_clock::now();
    const ScopeExit onScopeExit = [&]() noexcept
    {
        m_lastCameraPositionUpdatelTime = now;
    };

    const auto millisecondsElapsedSinceLastUpdate =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            now - m_lastCameraPositionUpdatelTime);

    HandleGamepadInput(millisecondsElapsedSinceLastUpdate);
    HandleKeyboardInput(millisecondsElapsedSinceLastUpdate);
}

void GLCanvas::HandleKeyboardInput(const std::chrono::milliseconds& elapsedTime)
{
    const bool isWKeyDown = m_keyboardManager.IsKeyDown(Qt::Key_W);
    const bool isAKeyDown = m_keyboardManager.IsKeyDown(Qt::Key_A);
    const bool isSKeyDown = m_keyboardManager.IsKeyDown(Qt::Key_S);
    const bool isDKeyDown = m_keyboardManager.IsKeyDown(Qt::Key_D);

    if ((isWKeyDown && isSKeyDown) || (isAKeyDown && isDKeyDown)) {
        return;
    }

    const auto millisecondsElapsed = elapsedTime.count();
    const auto cameraSpeed = m_controller.GetSettingsManager().GetCameraSpeed();

    if (isWKeyDown) {
        m_camera.OffsetPosition(
            static_cast<float>(millisecondsElapsed * cameraSpeed) * m_camera.Forward());
    }

    if (isAKeyDown) {
        m_camera.OffsetPosition(
            static_cast<float>(millisecondsElapsed * cameraSpeed) * m_camera.Left());
    }

    if (isSKeyDown) {
        m_camera.OffsetPosition(
            static_cast<float>(millisecondsElapsed * cameraSpeed) * m_camera.Backward());
    }

    if (isDKeyDown) {
        m_camera.OffsetPosition(
            static_cast<float>(millisecondsElapsed * cameraSpeed) * m_camera.Right());
    }
}

void GLCanvas::HandleGamepadInput(const std::chrono::milliseconds& elapsedTime)
{
    if (!m_mainWindow.GetGamepad().isConnected()) {
        return;
    }

    const auto& gamepad = m_mainWindow.GetGamepad();

    HandleGamepadButtonInput(gamepad, elapsedTime);
    HandleGamepadThumbstickInput(gamepad);
    HandleGamepadTriggerInput(gamepad);
}

void GLCanvas::HandleGamepadButtonInput(
    const Gamepad& gamepad, const std::chrono::milliseconds& elapsedTime)
{
    const auto millisecondsElapsed = elapsedTime.count();
    const auto cameraSpeed = m_controller.GetSettingsManager().GetCameraSpeed() /
                             Constants::Input::MovementAmplification;

    if (gamepad.buttonUp()) {
        m_camera.OffsetPosition(
            static_cast<float>(millisecondsElapsed * cameraSpeed) * m_camera.Forward());
    }

    if (gamepad.buttonLeft()) {
        m_camera.OffsetPosition(
            static_cast<float>(millisecondsElapsed * cameraSpeed) * m_camera.Left());
    }

    if (gamepad.buttonDown()) {
        m_camera.OffsetPosition(
            static_cast<float>(millisecondsElapsed * cameraSpeed) * m_camera.Backward());
    }

    if (gamepad.buttonRight()) {
        m_camera.OffsetPosition(
            static_cast<float>(millisecondsElapsed * cameraSpeed) * m_camera.Right());
    }

    if (gamepad.buttonL1()) {
        m_camera.OffsetPosition(
            static_cast<float>(millisecondsElapsed * cameraSpeed) * m_camera.Down());
    }

    if (gamepad.buttonR1()) {
        m_camera.OffsetPosition(
            static_cast<float>(millisecondsElapsed * cameraSpeed) * m_camera.Up());
    }

    if (!m_gamepadContextMenu && gamepad.buttonA()) {
        ShowGamepadContextMenu();
    } else if (m_gamepadContextMenu && !gamepad.buttonA()) {
        m_gamepadContextMenu->ExecuteSelection();

        m_gamepadContextMenu->close();
        m_gamepadContextMenu = nullptr;
    }
}

void GLCanvas::HandleGamepadThumbstickInput(const Gamepad& gamepad)
{
    if (m_gamepadContextMenu) {
        return;
    }

    if (gamepad.axisRightX() > 0.0 || gamepad.axisRightY() > 0.0) {
        const auto pitch = Constants::Input::MovementAmplification *
                           m_controller.GetSettingsManager().GetMouseSensitivity() *
                           gamepad.axisRightY();

        const auto yaw = Constants::Input::MovementAmplification *
                         m_controller.GetSettingsManager().GetMouseSensitivity() *
                         gamepad.axisRightX();

        m_camera.OffsetOrientation(pitch, yaw);
    }

    if (gamepad.axisLeftY() > 0.0) {
        m_camera.OffsetPosition(
            static_cast<float>(
                Constants::Input::MovementAmplification *
                m_controller.GetSettingsManager().GetCameraSpeed() * -gamepad.axisLeftY()) *
            m_camera.Forward());
    }

    if (gamepad.axisLeftX() > 0.0) {
        m_camera.OffsetPosition(
            static_cast<float>(
                Constants::Input::MovementAmplification *
                m_controller.GetSettingsManager().GetCameraSpeed() * gamepad.axisLeftX()) *
            m_camera.Right());
    }
}

void GLCanvas::HandleGamepadTriggerInput(const Gamepad& gamepad)
{
    if (!m_isLeftTriggerDown && gamepad.IsLeftTriggerDown()) {
        m_isLeftTriggerDown = true;

        auto* const crosshair = GetAsset<Assets::Tag::Crosshair>();
        crosshair->SetCrosshairLocation(m_camera.GetViewport().center());
        crosshair->Show();
    } else if (m_isLeftTriggerDown && !gamepad.IsLeftTriggerDown()) {
        m_isLeftTriggerDown = false;

        auto* const crosshair = GetAsset<Assets::Tag::Crosshair>();
        crosshair->Hide();
    }

    if (!m_isRightTriggerDown && gamepad.IsRightTriggerDown()) {
        m_isRightTriggerDown = true;

        SelectNodeViaRay(m_camera.GetViewport().center());
    } else if (m_isRightTriggerDown && !gamepad.IsRightTriggerDown()) {
        m_isRightTriggerDown = false;
    }
}

void GLCanvas::SelectNodeViaRay(const QPoint& rayOrigin)
{
    const auto selectionCallback = [&](auto& node) { SelectNode(node); };
    const auto deselectionCallback = [&](auto& node) { RestoreSelectedNode(node); };

    const auto ray = m_camera.ShootRayIntoScene(rayOrigin);
    m_controller.SelectNodeViaRay(m_camera, ray, deselectionCallback, selectionCallback);
}

void GLCanvas::UpdateFrameTime(const std::chrono::microseconds& elapsedTime)
{
    constexpr auto movingAverageWindowSize{ 64 };
    if (m_frameTimeDeque.size() > movingAverageWindowSize) {
        m_frameTimeDeque.pop_front();
    }

    Expects(m_frameTimeDeque.size() <= movingAverageWindowSize);
    m_frameTimeDeque.emplace_back(static_cast<int>(elapsedTime.count()));

    const auto total = std::accumulate(
        std::begin(m_frameTimeDeque), std::end(m_frameTimeDeque), 0ull,
        [](const auto runningTotal, const auto frameTime) noexcept {
            return runningTotal + frameTime;
        });

    Expects(m_frameTimeDeque.empty() == false);
    const auto averageFrameTime = total / m_frameTimeDeque.size();

    const auto label = L"D-Viz @ " + std::to_wstring(averageFrameTime) + L" \xB5s / frame";
    m_mainWindow.setWindowTitle(QString::fromStdWString(label));
}

void GLCanvas::VisualizeFilesystemActivity()
{
    if (!m_controller.HasModelBeenLoaded() || !m_controller.IsFileSystemBeingMonitored()) {
        return;
    }

    auto notification = m_controller.FetchFileModification();
    Assets::Treemap* treemap = notification ? GetAsset<Assets::Tag::Treemap>() : nullptr;

    const auto markNode = [&](const Tree<VizBlock>::Node& node, const QVector3D& color) {
        if (!m_controller.GetSettingsManager().ShouldBlockBeProcessed(node.GetData())) {
            return;
        }

        m_controller.RegisterNodeColor(node, color);
        treemap->SetNodeColor(node, color);
    };

    const auto startTime = std::chrono::high_resolution_clock::now();

    while (notification) {
        const ScopeExit onScopeExit = [&]() noexcept
        {
            notification = m_controller.FetchFileModification();
        };

        const auto* affectedNode = Utilities::FindNodeUsingRelativePath(
            m_controller.GetTree().GetRoot(), notification->path);

        if (affectedNode == nullptr) {
            // @note Since files may have been created after the latest scan, it is possible for an
            // event to not have an associated node in the tree.
            continue;
        }

        if (notification->eventType == FileEventType::TOUCHED) {
            markNode(*affectedNode, Constants::Colors::BabyBlue);
        } else if (notification->eventType == FileEventType::DELETED) {
            if (affectedNode->GetData().file.type == FileType::DIRECTORY) {
                // @todo If a directory is deleted via the Windows File Explorer, no notifications
                // are sent for any file that resides below that directory. Investigate if this
                // behavior is Windows specific.
                std::for_each(
                    Tree<VizBlock>::PostOrderIterator{ affectedNode },
                    Tree<VizBlock>::PostOrderIterator{},
                    [&](const auto& node) { markNode(node, Constants::Colors::HotPink); });
            } else {
                markNode(*affectedNode, Constants::Colors::HotPink);
            }
        }

        const auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - startTime);

        constexpr auto timeoutValue = Constants::Graphics::DesiredTimeBetweenFrames / 2;
        constexpr auto timeLimit = std::chrono::milliseconds{ timeoutValue };
        if (elapsedTime >= timeLimit) {
            // @note Since this processing is happening on the UI thread, we'll want to make sure
            // that we don't exceed a reasonable fraction of the total allotted frame time.
            break;
        }
    }
}

void GLCanvas::paintGL()
{
    if (m_isPaintingSuspended) {
        return;
    }

    const auto stopwatch = Stopwatch<std::chrono::microseconds>([&]() noexcept {
        VisualizeFilesystemActivity();

        m_openGLContext.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (m_controller.GetSettingsManager().IsPrimaryLightAttachedToCamera()) {
            Expects(m_lights.empty() == false);
            m_lights.front().position = m_camera.GetPosition();
        }

        for (const auto& tagAndAsset : m_sceneAssets) {
            tagAndAsset.asset->Render(m_camera, m_lights);
        }
    });

    if (m_mainWindow.ShouldShowFrameTime()) {
        UpdateFrameTime(stopwatch.GetElapsedTime());
    }
}

template void GLCanvas::ToggleAssetVisibility<Assets::Tag::Grid>(bool) const noexcept;
template void GLCanvas::ToggleAssetVisibility<Assets::Tag::LightMarker>(bool) const noexcept;
template void GLCanvas::ToggleAssetVisibility<Assets::Tag::OriginMarker>(bool) const noexcept;
