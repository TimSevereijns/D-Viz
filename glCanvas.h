#ifndef GLCANVAS_H
#define GLCANVAS_H

#include "camera.h"
#include "keyboardManager.h"
#include "mainwindow.h"
#include "optionsManager.h"

#include "DataStructs/light.h"
#include "DriveScanner/driveScanner.h"
#include "Scene/sceneAsset.h"
#include "Visualizations/visualization.h"

#include <chrono>
#include <memory>

#include <QOpenGLBuffer>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QTimer>
#include <QVector3D>

/**
 * @brief The GLCanvas class represents the canvas object on which the visualization
 * is to be drawn. This class contains all the shader programs, Vertex Buffer Objects
 * (VBOs), lights, and other assets needed to represent the scene.
 */
class GLCanvas : public QOpenGLWidget, protected QOpenGLFunctions
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
       * TODO: Expose the camera object from this class.
       * 
       * @param[in] fieldOfView  The new field of view.
       */
      void SetFieldOfView(const float fieldOfView);

      void HandleRightClick(const QMouseEvent& event);

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

      std::chrono::time_point<std::chrono::system_clock> UpdateFPS();

      void HandleInput();
      void HandleXBoxControllerInput();

      void PrepareVisualizationShaderProgram();
      void PrepareOriginMarkerShaderProgram();

      void PrepareVisualizationVertexBuffers();
      void PrepareOriginMarkerVertexBuffers();

      bool m_isPaintingSuspended;
      bool m_isVisualizationLoaded;

      double m_distance;

      MainWindow* m_mainWindow;

      std::unique_ptr<Visualization> m_theVisualization;
      std::unique_ptr<QTimer> m_frameRedrawTimer;

      Camera m_camera;

      DriveScanner m_scanner;

      Light m_light;

      KeyboardManager m_keyboardManager;

      std::shared_ptr<OptionsManager> m_settings;

      QMatrix4x4 m_projectionMatrix;

      std::vector<std::unique_ptr<SceneAsset>> m_sceneAssets;

      QOpenGLShaderProgram m_originMarkerShaderProgram;
      QOpenGLShaderProgram m_visualizationShaderProgram;

      QOpenGLVertexArrayObject m_visualizationVAO;
      QOpenGLVertexArrayObject m_originMarkerVAO;

      QOpenGLBuffer m_visualizationVertexPositionBuffer;
      QOpenGLBuffer m_visualizationVertexColorBuffer;

      QOpenGLBuffer m_originMarkerVertexPositionBuffer;
      QOpenGLBuffer m_originMarkerVertexColorBuffer;

      QVector<QVector3D> m_originMarkerVertices;
      QVector<QVector3D> m_originMarkerColors;

      QVector<QVector3D> m_visualizationVertices;
      QVector<QVector3D> m_visualizationColors;

      std::chrono::system_clock::time_point m_lastFrameTimeStamp;

      QPoint m_lastMousePosition;
};

#endif // GLCANVAS_H
