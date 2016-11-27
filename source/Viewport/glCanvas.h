#ifndef GLCANVAS_H
#define GLCANVAS_H

#include "camera.h"
#include "controller.h"
#include "DataStructs/light.h"
#include "DriveScanner/driveScanner.h"
#include "HID/keyboardManager.h"
#include "optionsManager.h"
#include "Scene/sceneAsset.h"
#include "Viewport/graphicsDevice.h"
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
class GLCanvas : public QOpenGLWidget
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
      inline void SelectNode(const TreeNode<VizNode>* const node);

      /**
       * @brief Restores the color of the selected node back to its unselected state.
       */
      void RestoreSelectedNode();

      /**
       * @brief HighlightSelectedNodes
       */
      void HighlightNodes(std::vector<const TreeNode<VizNode>*>& nodes);

      /**
       * @brief Returns any highlighted nodes back to their unhighlighted colors.
       */
      void RestoreHighlightedNodes(std::vector<const TreeNode<VizNode>*>& nodes);

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
       * @brief Handles keyboard, mouse, and Xbox controller input.
       */
      void HandleInput();

   private:

      /**
       * @brief Computes and updates the running average of the visualization's frame rate.
       */
      void UpdateFPS();

      /**
       * @brief Generates and displays the context menu.
       *
       * @param[in] point           The location at which to place the top left corner of the menu.
       */
      void ShowContextMenu(const QPoint& point);

      /**
       * @brief Handles the input from the Xbox controller.
       */
      void HandleXBoxControllerInput();

      /**
       * @brief Handles Xbox left and right trigger input.
       *
       * @param[in] controllerState    The current state of the controller.
       */
      void HandleXboxTriggerInput(const XboxController::State& controllerState);

      /**
       * @brief Handles Xbox thumb stick input.
       *
       * @param[in] controllerState    The current state of the controller.
       */
      void HandleXboxThumbstickInput(const XboxController::State& controllerState);

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

      std::unique_ptr<GraphicsDevice> m_graphicsDevice{ nullptr };

      std::unique_ptr<QTimer> m_frameRedrawTimer{ nullptr };
      std::unique_ptr<QTimer> m_inputCaptureTimer{ nullptr };

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

      std::deque<int> m_frameRateDeque;
};

#endif // GLCANVAS_H
