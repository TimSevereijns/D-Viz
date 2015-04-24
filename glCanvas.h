#ifndef GLCANVAS_H
#define GLCANVAS_H

#include "camera.h"
#include "keyboardManager.h"
#include "mainwindow.h"
#include "Visualizations/visualization.h"

#include <chrono>
#include <memory>

#include <QOpenGLBuffer>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QVector3D>

/**
 * @brief The Light struct
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
        attenuation(0.75f),     // Arbitrary coefficient to control attenuation beyond distance.
        ambientCoefficient(0.01f) // Minimum brightness is 1% of maximum brightness
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

   void SetPosition(const QVector3D& newPosition)
   {
      position = newPosition;
   }
};

/**
 * @brief The GLCanvas class
 */
class GLCanvas : public QOpenGLWidget, protected QOpenGLFunctions
{
   Q_OBJECT

   public:
      explicit GLCanvas(QWidget* parent = nullptr);
      ~GLCanvas();

      /**
       * @brief ParseVisualization
       * @param path
       * @param options
       */
      void ParseVisualization(const std::wstring& path, const ParsingOptions& options);

      /**
       * @brief setFieldOfView
       * @param fov
       */
      void SetFieldOfView(const float fieldOfView);

   public slots:
      /**
       * @brief OnCameraMovementSpeedChanged
       * @param newSpeed
       */
      void OnCameraMovementSpeedChanged(const double newSpeed);

      /**
       * @brief OnMouseSensitivityChanged
       * @param newSensitivity
       */
      void OnMouseSensitivityChanged(const double newSensitivity);

      /**
       * @brief OnAmbientCoefficientChanged
       * @param newCoefficient
       */
      void OnAmbientCoefficientChanged(const double newCoefficient);

      /**
       * @brief OnAttenuationChanged
       * @param newAttenuation
       */
      void OnAttenuationChanged(const double newAttenuation);

      /**
       * @brief OnShiniessChanged
       * @param newShininess
       */
      void OnShininessChanged(const double newShininess);

      /**
       * @brief OnRedLightComponentChanged
       * @param value
       */
      void OnRedLightComponentChanged(const int value);

      /**
       * @brief OnGreenLightComponentChanged
       * @param value
       */
      void OnGreenLightComponentChanged(const int value);

      /**
       * @brief OnBlueLightComponentChanged
       * @param value
       */
      void OnBlueLightComponentChanged(const int value);

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
      void HandleCameraMovement();

      void PrepareVisualizationShaderProgram();
      void PrepareOriginMarkerShaderProgram();

      void PrepareVisualizationVertexBuffers();
      void PrepareOriginMarkerVertexBuffers();

      void PrepareFXAAShaderProgram();

      void ApplyFXAA(int level);

      bool m_isPaintingSuspended;
      bool m_isVisualizationLoaded;

      double m_distance;
      double m_cameraMovementSpeed;
      double m_mouseSensitivity;

      float m_ambientCoefficient;
      float m_attenuation;
      float m_materialShininess;

      float m_redLightComponent;
      float m_greenLightComponent;
      float m_blueLightComponent;

      std::wstring m_visualizedDirectory;

      MainWindow* m_mainWindow;

      std::unique_ptr<Visualization> m_treeMap;

      Camera m_camera;

      Light m_light;

      KeyboardManager m_keyboardManager;

      QMatrix4x4 m_projectionMatrix;

      QOpenGLShaderProgram m_originMarkerShaderProgram;
      QOpenGLShaderProgram m_visualizationShaderProgram;
      QOpenGLShaderProgram m_FXAAShaderProgram;

      QOpenGLVertexArrayObject m_visualizationVAO;
      QOpenGLVertexArrayObject m_originMarkerVAO;

      // Needs to be a pointer, since we need to first construct a valid OpenGL context before we
      // create the framebuffer.
      QOpenGLFramebufferObject* m_frameBuffer;
      QOpenGLFramebufferObject* m_screenBuffer;

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
