#ifndef GLCANVAS_H
#define GLCANVAS_H

#include "camera.h"
#include "controller.h"
#include "DataStructs/light.h"
#include "DriveScanner/driveScanner.h"
#include "HID/keyboardManager.h"
#include "optionsManager.h"
#include "Scene/sceneAsset.h"
#include "Visualizations/visualization.h"
#include "Windows/mainWindow.h"

#include <chrono>
#include <deque>
#include <memory>

#include <QOpenGLWidget>
#include <QTimer>
#include <QVector3D>

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
      GLCanvas(
         Controller& controller,
         QWidget* parent = nullptr);

      /**
       * @brief Reloads the current visualization.
       *
       * @param[in] parameters      @see VisualizationParameters
       */
      void ReloadVisualization();

      /**
       * @brief Sets the current field of view for the camera.
       *
       * @param[in] fieldOfView     The new field of view.
       */
      void SetFieldOfView(const float fieldOfView);

      /**
       * @brief Paints the selected node.
       *
       * @param[in] node            The node whose visual representation is to be repainted.
       */
      inline void SelectNode(const Tree<VizFile>::Node* const node);

      /**
       * @brief Restores the color of the selected node back to its unselected state.
       */
      void RestoreSelectedNode();

      /**
       * @brief HighlightSelectedNodes
       */
      void HighlightNodes(std::vector<const Tree<VizFile>::Node*>& nodes);

      /**
       * @brief Returns any highlighted nodes back to their unhighlighted colors.
       */
      void RestoreHighlightedNodes(std::vector<const Tree<VizFile>::Node*>& nodes);

   protected:

      void initializeGL() override;
      void resizeGL(int width, int height) override;
      void paintGL() override;

      void keyPressEvent(QKeyEvent* event) override;
      void keyReleaseEvent(QKeyEvent* event) override;

      void mouseReleaseEvent(QMouseEvent *) override;
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
       *
       * @todo Double-check that this is actually a sane way of doing it, or if I'm missing
       * something.
       */
      void UpdateFrameTime(const std::chrono::microseconds& elapsedTime);

      /**
       * @brief Generates and displays the context menu.
       *
       * @param[in] point           The location at which to place the top left corner of the menu.
       */
      void ShowContextMenu(const QPoint& point);

      /**
       * @brief HandleKeyboardInput
       *
       * @param[in] elapsedTime
       */
      void HandleKeyboardInput(const std::chrono::milliseconds& elapsedTime);

      /**
       * @brief Handles the input from the gamepad controller.
       *
       * @param[in] elapsedTime
       */
      void HandleGamepadInput(const std::chrono::milliseconds& elapsedTime);

      /**
       * @brief HandleGamepadKeyInput
       *
       * @param gamepad
       * @param elapsedTime
       */
      void HandleGamepadKeyInput(
         const Gamepad& gamepad,
         const std::chrono::milliseconds& elapsedTime);

      /**
       * @brief Handles left and right trigger input.
       *
       * @param[in] gamepad
       */
      void HandleGamepadTriggerInput(const Gamepad& gamepad);

      /**
       * @brief Handles thumb stick inputs.
       *
       * @param[in] gamepad.
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
       * @brief SelectNodeViaRay
       */
      void SelectNodeViaRay(const QPoint& rayOrigin);

      bool m_isPaintingSuspended{ false };
      bool m_isVisualizationLoaded{ false };
      bool m_isLeftTriggerDown{ false };
      bool m_isRightTriggerDown{ false };
      bool m_isLeftMouseButtonDown{ false };
      bool m_isCursorHidden{ false };

      Controller& m_controller;

      MainWindow& m_mainWindow;

      QOpenGLExtraFunctions m_graphicsDevice;

      QTimer m_frameRedrawTimer{ nullptr };

      std::chrono::system_clock::time_point m_lastFrameDrawTime
      {
         std::chrono::system_clock::now()
      };

      std::chrono::system_clock::time_point m_lastCameraPositionUpdatelTime
      {
         std::chrono::system_clock::now()
      };

      std::chrono::system_clock::time_point m_startOfMouseLookEvent
      {
         std::chrono::system_clock::now()
      };

      std::vector<Light> m_lights
      {
         Light{ },
         Light{ QVector3D{ 0.0f, 80.0f, 0.0f } },
         Light{ QVector3D{ 0.0f, 80.0f, -VisualizationModel::ROOT_BLOCK_DEPTH } },
         Light{ QVector3D{ VisualizationModel::ROOT_BLOCK_WIDTH, 80.0f, 0.0f } },
         Light{ QVector3D{ VisualizationModel::ROOT_BLOCK_WIDTH, 80.0f, -VisualizationModel::ROOT_BLOCK_DEPTH } }
      };

      std::shared_ptr<OptionsManager> m_optionsManager;

      Camera m_camera;

      KeyboardManager m_keyboardManager;

      QMatrix4x4 m_projectionMatrix;

      QPoint m_lastMousePosition;

      std::vector<std::unique_ptr<SceneAsset>> m_sceneAssets;

      std::deque<int> m_frameTimeDeque;
};

#endif // GLCANVAS_H
