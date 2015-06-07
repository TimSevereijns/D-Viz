#ifndef GLCANVAS_H
#define GLCANVAS_H

#include "camera.h"
#include "keyboardManager.h"
#include "mainwindow.h"
#include "optionsManager.h"
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
 * @brief The Light struct represents a single point light. The light's fall-off with distance
 * can be tweaked via the attentuation attribute, and the light's contribution to the ambient
 * brightness of the scene can also be specified.
 */
struct Light
{
   QVector3D position;
   QVector3D intensity;
   float attenuation;
   float ambientCoefficient;

   Light()
      : position(QVector3D(0, 0, 0)),
        intensity(QVector3D(1, 1, 1)),
        attenuation(0.75f),         // Arbitrary coefficient to control attenuation beyond distance.
        ambientCoefficient(0.01f)   // Minimum brightness is 1% of maximum brightness
   {
   }

   Light(const QVector3D& lightPosition, const QVector3D& lightIntensity,
         const float lightAttenuation, const float lightAmbientCoefficient)
      : position(lightPosition),
        intensity(lightIntensity),
        attenuation(lightAttenuation),
        ambientCoefficient(lightAmbientCoefficient)
   {
   }
};

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
      ~GLCanvas();

      /**
       * @brief ParseVisualization
       * 
       * @param[in] path        The directory to visualize.
       * @param[in] options     A struct containing a variety of tweakable parameters.
       */
      void ParseVisualization(const std::wstring& path, const ParsingOptions& options);

      /**
       * @brief setFieldOfView sets the current field of view for the camera.
       * 
       * TODO: Expose the camera object from this class.
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
      void HandleInput();
      void HandleXBoxControllerInput();

      void PrepareVisualizationShaderProgram();
      void PrepareOriginMarkerShaderProgram();

      void PrepareVisualizationVertexBuffers();
      void PrepareOriginMarkerVertexBuffers();

      bool m_isPaintingSuspended;
      bool m_isVisualizationLoaded;

      double m_distance;

      std::wstring m_visualizedDirectory;

      MainWindow* m_mainWindow;

      std::unique_ptr<Visualization> m_treeMap;
      std::unique_ptr<QTimer> m_frameRedrawTimer;

      Camera m_camera;

      Light m_light;

      KeyboardManager m_keyboardManager;

      std::shared_ptr<OptionsManager> m_settings;

      QMatrix4x4 m_projectionMatrix;

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

   signals:
};

#endif // GLCANVAS_H
