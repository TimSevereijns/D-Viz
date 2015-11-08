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
#include <memory>
#include <deque>

#include <QOpenGLWidget>
#include <QTimer>
#include <QVector3D>

/**
 * @brief The GLCanvas class represents the canvas object on which the visualization
 * is to be drawn. This class contains the central rendering code that invokes the render functions
 * on the individual scene assets that make up the entire visualization.
 */
class GLCanvas : public QOpenGLWidget
{
   Q_OBJECT

   public:
      explicit GLCanvas(QWidget* parent = nullptr);

      /**
       * @brief CreateNewVisualization
       *
       * @param[in] parameters      @see VisualizationParameters
       */
      void CreateNewVisualization(const VisualizationParameters& parameters);

      /**
       * @brief ReloadVisualization
       *
       * @param[in] parameters      @see VisualizationParameters
       */
      void ReloadVisualization(const VisualizationParameters& parameters);

      /**
       * @brief setFieldOfView sets the current field of view for the camera.
       *
       * @param[in] fieldOfView  The new field of view.
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

   private:
      void ScanDrive(const VisualizationParameters& vizParameters);

      void UpdateFPS();

      void HandleRightClick(const QMouseEvent& event);

      void HandleInput();
      void HandleXBoxControllerInput();

      void PrepareVisualizationShaderProgram();
      void PrepareOriginMarkerShaderProgram();

      void PrepareVisualizationVertexBuffers();
      void PrepareOriginMarkerVertexBuffers();

      bool m_isPaintingSuspended;
      bool m_isVisualizationLoaded;

      VisualizationParameters m_visualizationParameters;

      MainWindow* m_mainWindow;

      std::unique_ptr<Visualization> m_theVisualization;
      std::unique_ptr<QTimer> m_frameRedrawTimer;

      std::shared_ptr<OptionsManager> m_settings;

      Camera m_camera;

      DriveScanner m_scanner;

      Light m_light;

      KeyboardManager m_keyboardManager;

      QMatrix4x4 m_projectionMatrix;

      QPoint m_lastMousePosition;

      std::vector<std::unique_ptr<SceneAsset>> m_sceneAssets;

      std::chrono::system_clock::time_point m_lastFrameTimeStamp;

      std::deque<const int> m_frameRateDeque;

      std::unique_ptr<GraphicsDevice> m_graphicsDevice;
};

#endif // GLCANVAS_H
