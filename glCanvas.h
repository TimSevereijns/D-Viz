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

   public slots:     // Move all of these functions to a unified options manager.
      /**
       * @brief OnCameraMovementSpeedChanged should be called when the camera's movement speed
       * changes.
       *
       * @param[in] newSpeed        The new speed.
       */
      //void OnCameraMovementSpeedChanged(const double newSpeed);

      /**
       * @brief OnMouseSensitivityChanged should be called when the mouse's movement sensitivity
       * changes.
       *
       * @param[in] newSensitivity  The new sensitivity value.
       */
      //void OnMouseSensitivityChanged(const double newSensitivity);

      /**
       * @brief OnAmbientCoefficientChanged should be called when the scene's minimum ambient
       * lighting changes.
       *
       * @param[in] newCoefficient  The new ambient lighting coefficient.
       */
      //void OnAmbientCoefficientChanged(const double newCoefficient);

      /**
       * @brief OnAttenuationChanged should be called when the point light's attentuation changes.
       *
       * @param[in] newAttenuation  The new attenuation factor.
       */
      //void OnAttenuationChanged(const double newAttenuation);

      /**
       * @brief OnShininessChanged should be called when the block material shininess changes.
       *
       * @param[in] newShininess    The new shininess value.
       */
      //void OnShininessChanged(const double newShininess);

      /**
       * @brief OnRedLightComponentChanged
       * @param value
       */
      //void OnRedLightComponentChanged(const int value);

      /**
       * @brief OnGreenLightComponentChanged
       * @param value
       */
      //void OnGreenLightComponentChanged(const int value);

      /**
       * @brief OnBlueLightComponentChanged
       * @param value
       */
      //void OnBlueLightComponentChanged(const int value);

      /**
       * @brief OnUseXBoxControllerStateChanged sets whether the XBox controller is to be used.
       *
       * @param[in] useController   Pass in true to enable the use of the XBox controller.
       */
      //void OnUseXBoxControllerStateChanged(const bool useController);

      /**
       * @brief OnAttachLightToCameraStateChanged
       * @param attached
       */
      //void OnAttachLightToCameraStateChanged(const bool attached);

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
      //bool m_useXBoxController;
      //bool m_isLightAttachedToCamera;

      double m_distance;
      //double m_cameraMovementSpeed;
      //double m_mouseSensitivity;

      //float m_ambientCoefficient;
      //float m_attenuation;
      //float m_materialShininess;

      //float m_redLightComponent;
      //float m_greenLightComponent;
      //float m_blueLightComponent;

      std::wstring m_visualizedDirectory;

      MainWindow* m_mainWindow;

      std::unique_ptr<Visualization> m_treeMap;

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
