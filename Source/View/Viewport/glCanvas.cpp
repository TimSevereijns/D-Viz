#include "View/Viewport/glCanvas.h"

#include "Model/squarifiedTreemap.h"
#include "Settings/visualizationOptions.h"
#include "Utilities/operatingSystem.h"
#include "Utilities/scopeExit.h"
#include "View/Viewport/gamepadContextMenu.h"
#include "View/Viewport/mouseContextMenu.h"
#include "View/mainWindow.h"
#include "constants.h"
#include "controller.h"

#include <QApplication>
#include <QMenu>
#include <QMessageBox>

#include <gsl/gsl_assert>
#include <stopwatch.h>

namespace
{
    /**
     * @brief Sets up the default scene lighting.
     *
     * @returns A vector of lights.
     */
    std::vector<Light> GetDefaultLights()
    {
        constexpr auto rootWidth = Constants::Treemap::RootBlockWidth;
        constexpr auto rootDepth = Constants::Treemap::RootBlockDepth;

        auto primaryLight =
            Light{ QVector3D{ 0, 400, 0 }, QVector3D{ 1.0f, 1.0f, 1.0f }, 0.75f, 0.01f };

        return { std::move(primaryLight), Light{ QVector3D{ -200.0f, 250.0f, 200.0f } },
                 Light{ QVector3D{ 0.0f, 80.0f, -rootDepth } },
                 Light{ QVector3D{ rootWidth, 80.0f, 0.0f } },
                 Light{ QVector3D{ rootWidth, 80.0f, -rootDepth } } };
    }

    /**
     * @brief Computes and sets the vertex and color data for the light markers.
     *
     * @param[in] lights                   The lights in the scene to be marked.
     * @param[out] lightMarkerAsset        The scene asset to be updated
     */
    void
    InitializeLightMarkers(const std::vector<Light>& lights, Assets::LightMarker& lightMarkerAsset)
    {
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

        constexpr auto verticesPerMarker = 6;
        for (std::size_t index = 0u; index < lights.size() * verticesPerMarker; ++index) {
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
                                   ? "Extensionless"
                                   : "\"" + node.GetData().file.extension + "\"";

        return QString::fromStdString("Highlight All " + extension + " Files");
    }

    /**
     * @brief Locates the node associated with a given file event notification.
     *
     * @param[in] notification      The file event notification to process.
     * @param[in] controller        The controller containing the treemap.
     */
    Tree<VizBlock>::Node* LocateNode(const FileEvent& notification, Controller& controller)
    {
        if (notification.path.is_absolute()) {
            return Utilities::FindNodeViaAbsolutePath(
                controller.GetTree().GetRoot(), notification.path);
        }

        return Utilities::FindNodeViaRelativePath(
            controller.GetTree().GetRoot(), notification.path);
    }
} // namespace

GLCanvas::GLCanvas(Controller& controller, QWidget* parent)
    : QOpenGLWidget{ parent },
      m_controller{ controller },
      m_mainWindow{ *(dynamic_cast<MainWindow*>(parent)) },
      m_lights{ GetDefaultLights() }
{
    m_camera.SetPosition({ 500, 100, 0 });
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
    const ScopeExit resetSuspension = [&]() noexcept
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
    Expects(event);

    if (event->isAutoRepeat()) {
        event->ignore();
        return;
    }

    m_keyboardManager.UpdateKeyState(
        static_cast<Qt::Key>(event->key()), KeyboardManager::KeyState::Down);

    event->accept();
}

void GLCanvas::keyReleaseEvent(QKeyEvent* const event)
{
    Expects(event);

    if (event->isAutoRepeat()) {
        event->ignore();
        return;
    }

    m_keyboardManager.UpdateKeyState(
        static_cast<Qt::Key>(event->key()), KeyboardManager::KeyState::Up);

    event->accept();
}

void GLCanvas::mousePressEvent(QMouseEvent* const event)
{
    Expects(event);

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
            m_startOfMouseLookEvent = std::chrono::steady_clock::now();
        }
    }

    event->accept();
}

void GLCanvas::mouseReleaseEvent(QMouseEvent* const event)
{
    Expects(event);

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
    Expects(event);

    const auto deltaX = event->x() - m_lastMousePosition.x();
    const auto deltaY = event->y() - m_lastMousePosition.y();

    if (!m_isCursorHidden) {
        m_lastMousePosition = event->pos();
    }

    if (event->buttons() & Qt::LeftButton) {
        m_camera.OffsetOrientation(
            m_controller.GetSessionSettings().GetMouseSensitivity() * deltaY,
            m_controller.GetSessionSettings().GetMouseSensitivity() * deltaX);

#ifdef Q_OS_UNIX
        // @note This only appears to work as expected on Linux. The camera angle jumps when the
        // cursor is hidden on Windows; I think there might still be messages in the event queue
        // that messes up the delta computation. @todo Investigate...

        const auto timeSinceStartOfLookEvent = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - m_startOfMouseLookEvent);

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
    Expects(event);

    event->accept();

    const auto delta = event->angleDelta().y();
    if (delta == 0) {
        return;
    }

    const auto cameraSpeed = m_controller.GetSessionSettings().GetCameraSpeed();

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
    treemap->SetNodeColor(node, Constants::Colors::Selected);
}

void GLCanvas::RestoreSelectedNode(const Tree<VizBlock>::Node& node)
{
    const auto& options = m_controller.GetSessionSettings().GetVisualizationOptions();
    if (!options.IsNodeVisible(node.GetData())) {
        return;
    }

    const auto restorationColor = m_controller.DetermineNodeColor(node);
    auto* const treemap = GetAsset<Assets::Tag::Treemap>();
    treemap->SetNodeColor(node, restorationColor);
}

void GLCanvas::HighlightNodes(std::vector<const Tree<VizBlock>::Node*>& nodes)
{
    auto* const treemap = GetAsset<Assets::Tag::Treemap>();
    for (const auto* const node : nodes) {
        treemap->SetNodeColor(*node, Constants::Colors::Highlighted);
    }
}

void GLCanvas::RestoreHighlightedNodes(std::vector<const Tree<VizBlock>::Node*>& nodes)
{
    auto* const treemap = GetAsset<Assets::Tag::Treemap>();
    const auto& options = m_controller.GetSessionSettings().GetVisualizationOptions();

    for (const auto* const node : nodes) {
        if (!options.IsNodeVisible(node->GetData())) {
            continue;
        }

        // @todo Is the color cleared appropriately in all cases...?
        const auto restorationColor = m_controller.DetermineNodeColor(*node);
        treemap->SetNodeColor(*node, restorationColor);
    }
}

bool GLCanvas::AlertIfMissing(const std::filesystem::path& path)
{
    if (std::filesystem::exists(path)) {
        return false;
    }

    m_mainWindow.DisplayErrorDialog("File no longer exists on disk.");
    return true;
}

template <typename MenuType>
void GLCanvas::AddOperatingSystemOptionsToContextMenu(
    MenuType& menu, FileType fileType, const Tree<VizBlock>::Node* const selection)
{
    Expects(selection);

    const auto path = Controller::NodeToFilePath(*selection);

    menu.addSeparator();

    menu.addAction("Copy File Name", [path] { OS::CopyFileNameToClipboard(path); });
    menu.addAction("Copy File Path", [path] { OS::CopyPathToClipboard(path); });

    menu.addSeparator();

    menu.addAction("Show in Explorer", [&, path] {
        if (AlertIfMissing(path)) {
            return;
        }

        OS::LaunchFileExplorer(path);
    });

    if (fileType == FileType::Regular) {
        menu.addAction("Open File", [&, path] {
            if (AlertIfMissing(path)) {
                return;
            }

            OS::OpenFile(path);
        });

        menu.addAction("Move to Trash", [&, path] {
            if (AlertIfMissing(path)) {
                return;
            }

            if (m_mainWindow.AskUserToConfirmDeletion(path)) {
                OS::MoveToTrash(path);
            }
        });
    }
}

template <typename MenuType> void GLCanvas::PopulateContextMenu(MenuType& menu)
{
    const auto unhighlightCallback = [&](auto& nodes) {
        RestoreHighlightedNodes(nodes);

        // Make sure to re-select the already selected node in case it was also part of the
        // highlighted set:
        const auto* const selectedNode = m_controller.GetSelectedNode();
        if (selectedNode) {
            SelectNode(*selectedNode);
        }
    };

    const auto highlightCallback = [&](auto& nodes) { HighlightNodes(nodes); };
    const auto selectionCallback = [&](auto& node) { SelectNode(node); };

    const auto thereExistHighlightedNodes = !m_controller.GetHighlightedNodes().empty();
    if (thereExistHighlightedNodes) {
        menu.addAction(
            "Clear Highlights", [=] { m_controller.ClearHighlightedNodes(unhighlightCallback); });
    }

    const auto* const selection = m_controller.GetSelectedNode();
    if (!selection) {
        return;
    }

    menu.addAction("Highlight Ancestors", [=] {
        m_controller.ClearHighlightedNodes(unhighlightCallback);
        m_controller.HighlightAncestors(*selection, highlightCallback);
    });

    menu.addAction("Highlight Descendants", [=] {
        m_controller.ClearHighlightedNodes(unhighlightCallback);
        m_controller.HighlightDescendants(*selection, highlightCallback);
    });

    const auto fileType = selection->GetData().file.type;
    if (fileType == FileType::Regular) {
        const auto message = GetHighlightExtensionLabel(*selection);
        menu.addAction(message, [=] {
            m_controller.ClearHighlightedNodes(unhighlightCallback);

            m_controller.HighlightAllMatchingExtensions(
                selection->GetData().file.extension, highlightCallback);

            m_controller.SelectNode(*selection, selectionCallback);
        });
    }

    AddOperatingSystemOptionsToContextMenu(menu, fileType, selection);
}

void GLCanvas::ShowGamepadContextMenu()
{
    // @note Qt will manage the lifetime of this object:
    m_gamepadContextMenu = new GamepadContextMenu{ m_mainWindow.GetGamepad(), this };
    PopulateContextMenu(*m_gamepadContextMenu);

    m_gamepadContextMenu->move(mapToGlobal(QPoint{ 0, 0 }));
    m_gamepadContextMenu->resize(width(), height());

    m_gamepadContextMenu->ComputeLayout();

    m_gamepadContextMenu->show();
    m_gamepadContextMenu->raise();
}

void GLCanvas::ShowContextMenu(const QPoint& point)
{
    MouseContextMenu menu{ m_keyboardManager };
    PopulateContextMenu(menu);

    const QPoint globalPoint = mapToGlobal(point);
    menu.exec(globalPoint);
}

void GLCanvas::HandleUserInput()
{
    const auto now = std::chrono::steady_clock::now();
    const ScopeExit recordTimestamp = [&]() noexcept
    {
        m_lastFrameUpdateTimestamp = now;
    };

    const auto millisecondsSinceLastFrame =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastFrameUpdateTimestamp);

    HandleGamepadInput(millisecondsSinceLastFrame);
    HandleKeyboardInput(millisecondsSinceLastFrame);
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
    const auto cameraSpeed = m_controller.GetSessionSettings().GetCameraSpeed();
    const auto distance = static_cast<float>(millisecondsElapsed * cameraSpeed);

    if (isWKeyDown) {
        m_camera.OffsetPosition(distance * m_camera.Forward());
    }

    if (isAKeyDown) {
        m_camera.OffsetPosition(distance * m_camera.Left());
    }

    if (isSKeyDown) {
        m_camera.OffsetPosition(distance * m_camera.Backward());
    }

    if (isDKeyDown) {
        m_camera.OffsetPosition(distance * m_camera.Right());
    }
}

void GLCanvas::HandleGamepadInput(const std::chrono::milliseconds& elapsedTime)
{
    const auto& gamepad = m_mainWindow.GetGamepad();

    if (!gamepad.isConnected()) {
        return;
    }

    HandleGamepadButtonInput(gamepad, elapsedTime);
    HandleGamepadThumbstickInput(gamepad);
    HandleGamepadTriggerInput(gamepad);
}

void GLCanvas::HandleGamepadButtonInput(
    const Gamepad& gamepad, const std::chrono::milliseconds& elapsedTime)
{
    const auto millisecondsElapsed = elapsedTime.count();
    const auto cameraSpeed = m_controller.GetSessionSettings().GetCameraSpeed() /
                             Constants::Input::MovementAmplification;

    const auto distance = static_cast<float>(millisecondsElapsed * cameraSpeed);

    if (gamepad.buttonUp()) {
        m_camera.OffsetPosition(distance * m_camera.Forward());
    }

    if (gamepad.buttonLeft()) {
        m_camera.OffsetPosition(distance * m_camera.Left());
    }

    if (gamepad.buttonDown()) {
        m_camera.OffsetPosition(distance * m_camera.Backward());
    }

    if (gamepad.buttonRight()) {
        m_camera.OffsetPosition(distance * m_camera.Right());
    }

    if (gamepad.buttonL1()) {
        m_camera.OffsetPosition(distance * m_camera.Down());
    }

    if (gamepad.buttonR1()) {
        m_camera.OffsetPosition(distance * m_camera.Up());
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

    if (gamepad.axisRightX() != 0.0 || gamepad.axisRightY() != 0.0) {
        const auto sensitivity = Constants::Input::MovementAmplification *
                                 m_controller.GetSessionSettings().GetMouseSensitivity();

        const auto pitch = sensitivity * gamepad.axisRightY();
        const auto yaw = sensitivity * gamepad.axisRightX();

        m_camera.OffsetOrientation(pitch, yaw);
    }

    const auto cameraSpeed = Constants::Input::MovementAmplification *
                             m_controller.GetSessionSettings().GetCameraSpeed();

    if (gamepad.axisLeftY() != 0.0) {
        const auto distance = static_cast<float>(cameraSpeed * -gamepad.axisLeftY());
        m_camera.OffsetPosition(distance * m_camera.Forward());
    }

    if (gamepad.axisLeftX() != 0.0) {
        const auto distance = static_cast<float>(cameraSpeed * gamepad.axisLeftX());
        m_camera.OffsetPosition(distance * m_camera.Right());
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

void GLCanvas::UpdateFrameTime(const std::chrono::milliseconds& elapsedTime)
{
    constexpr auto movingAverageWindowSize = 64;
    if (m_frameTimeDeque.size() > movingAverageWindowSize) {
        m_frameTimeDeque.pop_front();
    }

    Expects(m_frameTimeDeque.size() <= movingAverageWindowSize);
    m_frameTimeDeque.emplace_back(static_cast<int>(elapsedTime.count()));

    const auto total = std::accumulate(
        std::begin(m_frameTimeDeque), std::end(m_frameTimeDeque), 0,
        [](const auto runningTotal, const auto frameTime) noexcept {
            return runningTotal + frameTime;
        });

    Expects(m_frameTimeDeque.empty() == false);
    const auto averageFrameTime = total / static_cast<int>(m_frameTimeDeque.size());

    const auto label = "D-Viz @ " + std::to_string(averageFrameTime) + " ms / frame";
    m_mainWindow.setWindowTitle(QString::fromStdString(label));
}

void GLCanvas::PaintNode(
    Assets::Treemap* const treemap, const Tree<VizBlock>::Node& node, const QVector3D& fileColor,
    const QVector3D& directoryColor)
{
    const auto fileType = node.GetData().file.type;

    if (fileType == FileType::Regular) {
        m_controller.RegisterNodeColor(node, fileColor);
    } else {
        m_controller.RegisterNodeColor(node, directoryColor);
    }

    const auto& options = m_controller.GetSessionSettings().GetVisualizationOptions();
    if (!options.IsNodeVisible(node.GetData())) {
        return;
    }

    if (fileType == FileType::Regular) {
        treemap->SetNodeColor(node, fileColor);
    } else {
        treemap->SetNodeColor(node, directoryColor);
    }
}

void GLCanvas::HandleFileModification(
    Assets::Treemap* const treemap, const Tree<VizBlock>::Node& node)
{
    PaintNode(treemap, node, Constants::Colors::ModifiedFile, Constants::Colors::ModifiedDirectory);
}

void GLCanvas::HandleFileDeletion(Assets::Treemap* const treemap, const Tree<VizBlock>::Node& node)
{
    if (node.GetData().file.type == FileType::Directory) {
        // @todo If a directory is deleted via the Windows File Explorer, no
        // notifications are sent for any file that resides below that directory.
        // Investigate if this behavior is Windows specific.

        std::for_each(
            Tree<VizBlock>::PostOrderIterator{ &node }, Tree<VizBlock>::PostOrderIterator{},
            [&](const auto& child) {
                PaintNode(
                    treemap, child, Constants::Colors::DeletedFile,
                    Constants::Colors::DeletedDirectory);
            });
    } else {
        PaintNode(
            treemap, node, Constants::Colors::DeletedFile, Constants::Colors::DeletedDirectory);
    }
}

void GLCanvas::ProcessSingleFileEvent(
    const FileEvent& notification, Assets::Treemap* const treemap, const Tree<VizBlock>::Node& node)
{
    switch (notification.eventType) {
        case FileEventType::Touched: {
            HandleFileModification(treemap, node);
            break;
        }
        case FileEventType::Deleted: {
            HandleFileDeletion(treemap, node);
            break;
        }
        default: {
            break;
        }
    }
}

void GLCanvas::VisualizeFilesystemActivity()
{
    if (!m_controller.HasModelBeenLoaded() || !m_controller.IsFileSystemBeingMonitored()) {
        return;
    }

    std::optional<FileEvent> notification = m_controller.FetchNextFileModification();
    Assets::Treemap* const treemap = notification ? GetAsset<Assets::Tag::Treemap>() : nullptr;

    const auto startTime = std::chrono::steady_clock::now();

    while (notification) {
        const ScopeExit fetchNextNotification = [&]() noexcept
        {
            notification = m_controller.FetchNextFileModification();
        };

        const auto* const node = LocateNode(*notification, m_controller);
        if (node == nullptr) {
            // @note Since files may have been created after the latest scan, it is possible for an
            // event to not have an associated node in the tree.
            continue;
        }

        ProcessSingleFileEvent(*notification, treemap, *node);

        const auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startTime);

        constexpr auto timeLimit = std::chrono::milliseconds{ 16 };
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

    VisualizeFilesystemActivity();

    m_openGLContext.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_controller.GetSessionSettings().IsPrimaryLightAttachedToCamera()) {
        Expects(m_lights.empty() == false);
        m_lights.front().position = m_camera.GetPosition();
    }

    for (const auto& tagAndAsset : m_sceneAssets) {
        tagAndAsset.asset->Render(m_camera, m_lights);
    }

    if (m_mainWindow.ShouldShowFrameTime()) {
        const auto now = std::chrono::steady_clock::now();
        const auto elapsedTime =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastFrameDrawTime);

        m_lastFrameDrawTime = now;

        UpdateFrameTime(elapsedTime);
    }
}

template void GLCanvas::ToggleAssetVisibility<Assets::Tag::Grid>(bool) const noexcept;
template void GLCanvas::ToggleAssetVisibility<Assets::Tag::LightMarker>(bool) const noexcept;
template void GLCanvas::ToggleAssetVisibility<Assets::Tag::OriginMarker>(bool) const noexcept;
