#ifndef GLCANVAS_H
#define GLCANVAS_H

#include "Model/Scanner/driveScanner.h"
#include "Model/baseModel.h"
#include "Settings/persistentSettings.h"
#include "View/HID/keyboardManager.h"
#include "View/Scene/Assets/baseAsset.h"
#include "View/Scene/Assets/crosshairAsset.h"
#include "View/Scene/Assets/debuggingRayAsset.h"
#include "View/Scene/Assets/frustumAsset.h"
#include "View/Scene/Assets/gridAsset.h"
#include "View/Scene/Assets/lightMarkerAsset.h"
#include "View/Scene/Assets/originMarkerAsset.h"
#include "View/Scene/Assets/treemapAsset.h"
#include "View/Scene/light.h"
#include "camera.h"
#include "constants.h"
#include "gamepadContextMenu.h"

#include <chrono>
#include <deque>
#include <memory>

#include <QOpenGLWidget>
#include <QPainter>
#include <QTimer>
#include <QVector3D>

class Controller;
class Gamepad;
class MainWindow;

namespace Assets
{
    namespace Tag
    {
        struct Base
        {
            using AssetType = void;
            virtual int GetID() const noexcept
            {
                return 0;
            }

            virtual ~Base() noexcept = default;
        };

        struct OriginMarker final : Base
        {
            using AssetType = Assets::OriginMarker;
            int GetID() const noexcept final override
            {
                return 1;
            }
        };

        struct Grid final : Base
        {
            using AssetType = Assets::Grid;
            int GetID() const noexcept final override
            {
                return 2;
            }
        };

        struct Crosshair final : Base
        {
            using AssetType = Assets::Crosshair;
            int GetID() const noexcept final override
            {
                return 3;
            }
        };

        struct Treemap final : Base
        {
            using AssetType = Assets::Treemap;
            int GetID() const noexcept final override
            {
                return 4;
            }
        };

        struct LightMarker final : Base
        {
            using AssetType = Assets::LightMarker;
            int GetID() const noexcept final override
            {
                return 5;
            }
        };

        struct Frustum final : Base
        {
            using AssetType = Assets::Frustum;
            int GetID() const noexcept final override
            {
                return 6;
            }
        };
    } // namespace Tag
} // namespace Assets

struct TagAndAsset
{
    std::unique_ptr<Assets::Tag::Base> tag;
    std::unique_ptr<Assets::AssetBase> asset;
};

/**
 * @brief The GLCanvas class represents the canvas object on which the visualization is to be drawn.
 *
 * This class contains the central rendering code that invokes the render functions on the
 * individual scene assets that make up the entire visualization. Camera movement and scene
 * interaction are also handled by this class.
 */
class GLCanvas final : public QOpenGLWidget
{
    Q_OBJECT

  public:
    /**
     * @brief Constructs a new GLCanvas object.
     *
     * @param[in] controller      The Controller object that'll manage the visual representation
     *                            of the underlying treemap.
     * @param[in] parent          A pointer to the canvas's parent window.
     */
    GLCanvas(Controller& controller, QWidget* parent = nullptr);

    /**
     * @brief Reloads the current visualization.
     *
     * @param[in] parameters      @see VisualizationParameters
     */
    void ReloadVisualization();

    /**
     * @brief Applies the currently active color scheme, as set via the Settings::Manager.
     */
    void ApplyColorScheme();

    /**
     * @brief Sets the current field of view for the camera.
     *
     * @param[in] fieldOfView     The new field of view.
     */
    void SetFieldOfView(int fieldOfView);

    /**
     * @brief Paints the selected node.
     *
     * @param[in] node            The node whose visual representation is to be repainted.
     */
    inline void SelectNode(const Tree<VizBlock>::Node& node);

    /**
     * @brief Restores the color of the selected node back to its unselected state.
     *
     * @param[in] node            The node whose visual representation is to be repainted.
     */
    inline void RestoreSelectedNode(const Tree<VizBlock>::Node& node);

    /**
     * @brief HighlightSelectedNodes
     */
    void HighlightNodes(std::vector<const Tree<VizBlock>::Node*>& nodes);

    /**
     * @brief Returns any highlighted nodes back to their unhighlighted colors.
     */
    void RestoreHighlightedNodes(std::vector<const Tree<VizBlock>::Node*>& nodes);

    /**
     * @brief Toggles an asset's visibility.
     *
     * @param[in] shouldEnable    Pass in true if the asset should be visible.
     */
    template <typename TagType> void ToggleAssetVisibility(bool shouldEnable) const noexcept;

  protected:
    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

    void mouseReleaseEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

  private slots:

    /**
     * @brief Handles keyboard, mouse, and gamepad input, and also updates the OpenGL canvas.
     */
    void RunMainLoop();

  private:
    /**
     * @brief Handle Input
     */
    void HandleUserInput();

    /**
     * @brief Records the elapsed frame time.
     */
    void UpdateFrameTime(const std::chrono::milliseconds& elapsedTime);

    /**
     * @brief Generates and displays the context menu.
     *
     * @param[in] point           The location at which to place the top left corner of the menu.
     */
    void ShowContextMenu(const QPoint& point);

    /**
     * @brief Generates and displays a gamepad-compatible context menu.
     */
    void ShowGamepadContextMenu();

    /**
     * @brief Handles keyboard input.
     *
     * @param[in] elapsedTime     The time that has elapsed since the last processing of keyboard
     *                            state.
     */
    void HandleKeyboardInput(const std::chrono::milliseconds& elapsedTime);

    /**
     * @brief Handles the input from the gamepad controller.
     *
     * @param[in] elapsedTime     The time that has elapsed since the last processing of gamepad
     *                            state.
     */
    void HandleGamepadInput(const std::chrono::milliseconds& elapsedTime);

    /**
     * @brief Handles button input from the gamepad.
     *
     * @param[in] gamepad         The gamepad controller.
     * @param[in] elapsedTime     The time that has elapsed since the last processing of gamepad
     *                            state.
     */
    void
    HandleGamepadButtonInput(const Gamepad& gamepad, const std::chrono::milliseconds& elapsedTime);

    /**
     * @brief Handles left and right trigger input.
     *
     * @param[in] gamepad         The gamepad controller.
     */
    void HandleGamepadTriggerInput(const Gamepad& gamepad);

    /**
     * @brief Handles thumb stick inputs.
     *
     * @param[in] gamepad         The gamepad controller.
     */
    void HandleGamepadThumbstickInput(const Gamepad& gamepad);

    /**
     * @brief Compiles and loads the OpenGL shader program for the visualization.
     */
    void PrepareVisualizationShaderProgram();

    /**
     * @brief Compiles and loads the OpenGL shader program for the origin marker.
     */
    void PrepareOriginMarkerShaderProgram();

    /**
     * @brief Initializes the vertex buffer for the visualization.
     */
    void PrepareVisualizationVertexBuffers();

    /**
     * @brief Initializes the vertex buffer for the origin marker.
     */
    void PrepareOriginMarkerVertexBuffers();

    /**
     * @brief Uses a picking ray to select a node in the scene.
     *
     * @param[in] rayOrigin       The 2D coordinate on the OpenGL canvas from which the picking
     *                            ray will be projected into the scene.
     */
    void SelectNodeViaRay(const QPoint& rayOrigin);

    /**
     * @brief Helper function that turns scene asset retrieval into a simple one-liner.
     *
     * @tparam RequestedAsset     The tag specifying the type of the asset that is to be
     *                            retrieved. Note that we're implicitly assuming that we'll only
     *                            store one instance of each type.
     */
    template <typename RequestedAsset>
    typename RequestedAsset::AssetType* GetAsset() const noexcept;

    /**
     * @brief Helper function that turns scene asset registration into a simple one-liner.
     *
     * @tparam AssetTag           The tag specifying the type of asset to register. Note that
     *                            we're implicitly assuming that we'll only store one instance of
     *                            each type.
     */
    template <typename AssetTag> void RegisterAsset();

    /**
     * @brief Helper function that paints a node a particular color.
     *
     * @param[in] treemap           The treemap visualization asset.
     * @param[in] node              The node that needs painting.
     * @param[in] fileColor         The color to paint the node if it's a regular file.
     * @param[in] directoryColor    The color to paint the node if it's a directory.
     */
    void PaintNode(
        Assets::Treemap* const treemap, const Tree<VizBlock>::Node& node,
        const QVector3D& fileColor, const QVector3D& directoryColor);

    /**
     * @brief Helper function that handles the painting of node representing modified files.
     *
     * @param[in] treemap           The treemap visualization asset.
     * @param[in] node              The node that needs painting.
     */
    void HandleFileModification(Assets::Treemap* const treemap, const Tree<VizBlock>::Node& node);

    /**
     * @brief Helper function that handles the painting of node representing deleted files.
     *
     * @param[in] treemap           The treemap visualization asset.
     * @param[in] node              The node that needs painting.
     */
    void HandleFileDeletion(Assets::Treemap* const treemap, const Tree<VizBlock>::Node& node);

    /**
     * @brief Helper function to process a single file change event.
     *
     * @param[in] notification      The notification to handle.
     * @param[in] treemap           The treemap visualization asset.
     * @param[in] node              The node that needs painting.
     */
    void ProcessSingleFileEvent(
        const FileEvent& notification, Assets::Treemap* const treemap,
        const Tree<VizBlock>::Node& node);

    /**
     * @brief Updates the visualization to reflect recent filesystem activity.
     */
    void VisualizeFilesystemActivity();

    /**
     * @brief Helper function to add the OS options to the context menus.
     *
     * @param[in] menu              The menu that is to be populated.
     * @param[in] fileType          The type of file currently selected; it is assumed one is
     *                              selected.
     * @param[in] selection         A pointer to the node that is currently selected; a valid
     *                              selection is assumed.
     */
    template <typename MenuType>
    void AddOperatingSystemOptionsToContextMenu(
        MenuType& menu, FileType fileType, const Tree<VizBlock>::Node* const selection);

    /**
     * @brief Helper function populate the context menus.
     *
     * @param[in] menu              The menu that is to be populated.
     */
    template <typename MenuType> void PopulateContextMenu(MenuType& menu);

    /**
     * @brief Displays an error message if the file is missing from disk.
     *
     * @param[in] path              The path to test.
     *
     * @returns True if the file is missing, and false if present.
     */
    bool AlertIfMissing(const std::filesystem::path& path);

    bool m_isPaintingSuspended = false;
    bool m_isVisualizationLoaded = false;
    bool m_isLeftTriggerDown = false;
    bool m_isRightTriggerDown = false;
    bool m_isLeftMouseButtonDown = false;
    bool m_isCursorHidden = false;

    GamepadContextMenu* m_gamepadContextMenu = nullptr;

    Controller& m_controller;

    MainWindow& m_mainWindow;

    QOpenGLExtraFunctions m_openGLContext;

    QTimer m_frameRedrawTimer;

    std::chrono::steady_clock::time_point m_lastFrameDrawTime = std::chrono::steady_clock::now();

    std::chrono::steady_clock::time_point m_lastFrameUpdateTimestamp =
        std::chrono::steady_clock::now();

    std::chrono::steady_clock::time_point m_startOfMouseLookEvent =
        std::chrono::steady_clock::now();

    std::vector<Light> m_lights;

    Camera m_camera;

    KeyboardManager m_keyboardManager;

    QMatrix4x4 m_projectionMatrix;

    QPoint m_lastMousePosition;

    // @note Using an unsorted, linear container to store and retrieve assets is likely to
    // outperform std::unordered_map for a small number of assets. Should the asset count ever
    // grow past, say, 30 assets, then a std::unordered_map might start to make more sense.
    std::vector<TagAndAsset> m_sceneAssets;

    std::deque<int> m_frameTimeDeque;
};

#endif // GLCANVAS_H
