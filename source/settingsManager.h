#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include "settings.h"

#include <QObject>

namespace Settings
{
   /**
    * @brief The central class responsible for run-time settings.
    */
   class Manager final : public QObject
   {
      Q_OBJECT;

      public:

         Manager(const std::experimental::filesystem::path& colorConfigFile);

      public slots:

         /**
          * @brief Should be called when the camera's movement speed changes.
          *
          * @param[in] speed        The new speed.
          */
         void OnCameraSpeedChanged(double speed);

         /**
          * @brief Should be called when the mouse's movement sensitivity changes.
          *
          * @param[in] sensitivity  The new sensitivity value.
          */
         void OnMouseSensitivityChanged(double sensitivity);

         /**
          * @brief Should be called when the scene's minimum ambient lighting changes.
          *
          * @param[in] coefficient  The new ambient lighting coefficient.
          */
         void OnAmbientLightCoefficientChanged(double coefficient);

         /**
          * @brief Should be called when the point light's attentuation changes.
          *
          * @param[in] attenuation  The new attenuation factor.
          */
         void OnLightAttenuationChanged(double attenuation);

         /**
          * @brief Should be called when the block material shininess changes.
          *
          * @param[in] shininess    The new shininess value.
          */
         void OnMaterialShininessChanged(double shininess);

         /**
          * @brief Updates the attachment of the primary light to the camera.
          *
          * @param[in] attached        Pass in true to attach light number one to the camera.
          */
         void OnAttachLightToCameraStateChanged(bool attached);

         /**
          * @brief Handles changes in the field of view.
          *
          * @param[in] fieldOfView     Field of view in degrees.
          */
         void OnFieldOfViewChanged(int fieldOfView);

         /**
          * @brief OnShouldSearchFilesChanged
          *
          * @param[in] state           Pass in true if files should be searched for query matches.
          */
         void OnShouldSearchFilesChanged(bool state);

         /**
          * @brief OnShouldSearchDirectoriesChanged
          *
          * @param[in] state           Pass in true if directories should be searched for query
          *                            matches.
          */
         void OnShouldSearchDirectoriesChanged(bool state);

      public:

         double GetCameraSpeed() const;

         void SetCameraSpeed(double speed);

         double GetMouseSensitivity() const;

         float GetLightAttentuationFactor() const;

         float GetAmbientLightCoefficient() const;

         float GetMaterialShininess() const;

         bool IsPrimaryLightAttachedToCamera() const;

         const JsonDocument& GetFileColors() const;

         const VisualizationParameters& GetVisualizationParameters() const;

         void SetVisualizationParameters(const VisualizationParameters& parameters);

      private:

         double m_cameraSpeed{ 0.25 };
         double m_mouseSensitivity{ 0.20 };

         float m_ambientLightCoefficient{ 0.1f };
         float m_lightAttenuationFactor{ 0.005f };
         float m_materialShininess{ 80.0f };

         int m_fieldOfView{ 45 };

         bool m_isLightAttachedToCamera{ true };
         bool m_shouldSearchDirectories{ false };
         bool m_shouldSearchFiles{ true };

         JsonDocument m_fileColors;

         VisualizationParameters m_visualizationParameters;
   };
}

#endif //SETTINGSMANAGER_H
