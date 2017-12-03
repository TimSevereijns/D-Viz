#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include "constants.h"
#include "preferencesMap.hpp"
#include "settings.h"

#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <variant>

#include <QObject>
#include <QVector3D>

namespace Settings
{
   using ColorMap = std::unordered_map<std::wstring, std::unordered_map<std::wstring, QVector3D>>;

   /**
    * @brief The central class responsible for run-time settings.
    */
   class Manager final : public QObject
   {
      Q_OBJECT;

      public:

         Manager(
            const std::experimental::filesystem::path& colorFile,
            const std::experimental::filesystem::path& preferencesFile);

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

         QVector3D GetSpecularColor() const;

         bool IsPrimaryLightAttachedToCamera() const;

         const ColorMap& GetFileColorMap() const;

         const PreferencesMap& GetPreferenceMap() const;

         const std::wstring& GetActiveColorScheme() const;

         void SetColorScheme(const std::wstring& scheme);

         const VisualizationParameters& GetVisualizationParameters() const;

         VisualizationParameters& GetVisualizationParameters();

         VisualizationParameters& SetVisualizationParameters(
            const VisualizationParameters& parameters);

         void SetActiveNumericPrefix(Constants::FileSize::Prefix prefix);

         Constants::FileSize::Prefix GetActiveNumericPrefix() const;

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

         // @todo There's no real need to keep these around, I think...
         JsonDocument m_fileColorJsonDocument;
         JsonDocument m_generalSettingsJsonDocument;

         ColorMap m_colorMap;
         PreferencesMap m_preferencesMap;

         std::wstring m_colorScheme{ L"Default" };

         VisualizationParameters m_visualizationParameters;

         Constants::FileSize::Prefix m_activeNumericPrefix{ Constants::FileSize::Prefix::BINARY };
   };
}

#endif //SETTINGSMANAGER_H
