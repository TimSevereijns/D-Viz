#ifndef GLCANVAS_H
#define GLCANVAS_H

#include "camera.h"
#include "mainwindow.h"
#include "optionsManager.h"

#include "HID/keyboardManager.h"

#include "DataStructs/light.h"
#include "DriveScanner/driveScanner.h"
#include "Scene/sceneAsset.h"
#include "Viewport/graphicsDevice.h"
#include "Visualizations/visualization.h"

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
       * @brief Default constructor.
       *
       * @param[in] parent          Pointer to the parent UI element.
       */
      explicit GLCanvas(QWidget* parent = nullptr);

      /**
       * @brief Creates a new visualization.
       *
       * @param[in] parameters      @see VisualizationParameters
       */
      void CreateNewVisualization(VisualizationParameters& parameters);

      /**
       * @brief Reloads the current visualization.
       *
       * @param[in] parameters      @see VisualizationParameters
       */
      void ReloadVisualization(const VisualizationParameters& parameters);

      /**
       * @brief Sets the current field of view for the camera.
       *
       * @param[in] fieldOfView     The new field of view.
       */
      void SetFieldOfView(const float fieldOfView);

    protected:
      void initializeGL() override;
      void resizeGL(int width, int height) override;
      void paintGL() override;

      void keyPressEvent(QKeyEvent* event) override;
      void keyReleaseEvent(QKeyEvent* event) override;

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
       * @brief Initiates a drive scan.
       *
       * @param[in] vizParameters   @see VisualizationParameters
       */
      void ScanDrive(VisualizationParameters& vizParameters);

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
       * @brief Handles mouse right-click mouse events.
       *
       * @param[in] event           Details of the mouse click event.
       */
      void HandleRightClick(const QPoint& point);

      /**
       * @brief Handles TreeNode selection events.
       *
       * @param[in] selectedNode    Pointer to the TreeNode that the user clicked on, and nullptr if
       *                            the user's click didn't hit a TreeNode.
       */
      void HandleNodeSelection(const TreeNode<VizNode>* const selectedNode);

      /**
       * @brief Handles the input from the Xbox controller.
       */
      void HandleXBoxControllerInput();

      /**
       * @brief Handles Xbox left and right trigger input.
       *
       * @param[in] controllerState
       */
      void HandleXboxTriggerInput(const XboxController::State& controllerState);

      /**
       * @brief Handles Xbox thumb stick input.
       *
       * @param[in] controllerState
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
       * @brief Prompts the user if he or she would like to set a lower bound on which files are
       * visualized.
       *
       * @param[in] numberOfFilesScanned     The number of scanned files that can be visualized.
       * @param[in] parameters               @see VisualizationParameters
       */
      void AskUserToLimitFileSize(
         std::uintmax_t numberOfFilesScanned,
         VisualizationParameters& parameters) const;

      bool m_isPaintingSuspended{ false };
      bool m_isVisualizationLoaded{ false };

      bool m_isLeftTriggerDown{ false };
      bool m_isRightTriggerDown{ false };

      MainWindow* m_mainWindow{ nullptr };

      const TreeNode<VizNode>* m_selectedNode{ nullptr };
      const std::pair<TreeNode<VizNode>*, QVector3D> m_selectedNodeAndColor;

      std::unique_ptr<GraphicsDevice> m_graphicsDevice{ nullptr };

      std::unique_ptr<Visualization> m_theVisualization{ nullptr };

      std::unique_ptr<QTimer> m_frameRedrawTimer{ nullptr };
      std::unique_ptr<QTimer> m_cameraPositionTimer{ nullptr };

      std::chrono::system_clock::time_point m_lastFrameDrawTime
      {
         std::chrono::system_clock::now()
      };

      std::chrono::system_clock::time_point m_lastCameraPositionUpdatelTime
      {
         std::chrono::system_clock::now()
      };

      std::vector<Light> m_lights
      {
         Light{ },
         Light{ QVector3D{ 0.0f, 80.0f, 0.0f } },
         Light{ QVector3D{ 0.0f, 80.0f, -Visualization::ROOT_BLOCK_DEPTH } },
         Light{ QVector3D{ Visualization::ROOT_BLOCK_WIDTH, 80.0f, 0.0f } },
         Light{ QVector3D{ Visualization::ROOT_BLOCK_WIDTH, 80.0f, -Visualization::ROOT_BLOCK_DEPTH } }
      };

     VisualizationParameters m_visualizationParameters;

      std::shared_ptr<OptionsManager> m_settings;

      Camera m_camera;

      DriveScanner m_scanner;

      KeyboardManager m_keyboardManager;

      QMatrix4x4 m_projectionMatrix;

      QPoint m_lastMousePosition;

      std::vector<std::unique_ptr<SceneAsset>> m_sceneAssets;

      std::deque<int> m_frameRateDeque;
};

#endif // GLCANVAS_H
