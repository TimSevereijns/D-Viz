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

struct VizBlock;

namespace Settings
{
   using ColorMap = std::unordered_map<std::wstring, std::unordered_map<std::wstring, QVector3D>>;

   /**
    * @brief The central class responsible for run-time settings.
    */
   class Manager final : public QObject
   {
      Q_OBJECT

   public:

      Manager(
         const std::experimental::filesystem::path& colorFile,
         const std::experimental::filesystem::path& preferencesFile);

      /**
       * @brief GetCameraSpeed
       * @return
       */
      double GetCameraSpeed() const;

      /**
       * @brief SetCameraSpeed
       * @param speed
       */
      void SetCameraSpeed(double speed);

      /**
       * @brief GetMouseSensitivity
       * @return
       */
      double GetMouseSensitivity() const;

      /**
       * @brief GetLightAttentuationFactor
       * @return
       */
      double GetLightAttentuationFactor() const;

      /**
       * @brief GetAmbientLightCoefficient
       * @return
       */
      double GetAmbientLightCoefficient() const;

      /**
       * @brief GetMaterialShininess
       * @return
       */
      double GetMaterialShininess() const;

      /**
       * @brief GetSpecularColor
       * @return
       */
      QVector3D GetSpecularColor() const;

      /**
       * @brief IsPrimaryLightAttachedToCamera
       * @return
       */
      bool IsPrimaryLightAttachedToCamera() const;

      /**
       * @brief GetFileColorMap
       * @return
       */
      const ColorMap& GetFileColorMap() const;

      /**
       * @brief GetPreferenceMap
       * @return
       */
      const PreferencesMap& GetPreferenceMap() const;

      /**
       * @brief GetActiveColorScheme
       * @return
       */
      const std::wstring& GetActiveColorScheme() const;

      /**
       * @brief SetColorScheme
       * @param scheme
       */
      void SetColorScheme(const std::wstring& scheme);

      /**
       * @brief GetVisualizationParameters
       * @return
       */
      const VisualizationParameters& GetVisualizationParameters() const;

      /**
       * @brief GetVisualizationParameters
       * @return
       */
      VisualizationParameters& GetVisualizationParameters();

      /**
       * @brief SetVisualizationParameters
       * @param parameters
       * @return
       */
      VisualizationParameters& SetVisualizationParameters(
         const VisualizationParameters& parameters);

      /**
       * @brief SetActiveNumericPrefix
       * @param prefix
       */
      void SetActiveNumericPrefix(Constants::FileSize::Prefix prefix);

      /**
       * @brief GetActiveNumericPrefix
       * @return
       */
      Constants::FileSize::Prefix GetActiveNumericPrefix() const;

      /**
       * @brief ShouldShowCascadeSplitOverlay
       * @return
       */
      bool ShouldShowCascadeSplits() const;

      /**
       * @brief ShouldShowShadows
       * @return
       */
      bool ShouldRenderShadows() const;

      /**
       * @brief SaveChangeToDisk
       *
       * @todo Make private (will have to move some other functionality around as well).
       *
       * @param property
       * @param value
       *
       * @return
       */
      template<typename PropertyValueType>
      bool SavePreferenceChangeToDisk(
         std::wstring_view property,
         const PropertyValueType& value)
      {
         if (m_preferencesDocument.HasMember(property.data()))
         {
            m_preferencesDocument[property.data()] = value;
         }
         else
         {
            auto& allocator = m_preferencesDocument.GetAllocator();

            rapidjson::GenericValue<rapidjson::UTF16<wchar_t>> key{ property.data(), allocator };
            m_preferencesDocument.AddMember(key.Move(), value, allocator);
         }

         return Settings::SaveToDisk(m_preferencesDocument, m_preferencesPath);
      }

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

      /**
       * @brief OnShowCascadeSplitsToggled
       * @param isEnabled
       */
      void OnShowCascadeSplitsToggled(bool isEnabled);

      /**
       * @brief OnShowShadowsToggled
       * @param isEnabled
       */
      void OnShowShadowsToggled(bool isEnabled);

      /**
       * @brief OnMonitoringOptionToggled
       * @param isEnabled
       */
      void OnMonitoringOptionToggled(bool isEnabled);

      /**
       * @brief ShouldNodeBeIgnored
       * @return
       */
      bool ShouldBlockBeProcessed(const VizBlock& block);

   private:

      double m_cameraSpeed{ 0.25 };
      double m_mouseSensitivity{ 0.20 };

      double m_ambientLightCoefficient{ 0.2 };
      double m_lightAttenuationFactor{ 0.002 };
      double m_materialShininess{ 80.0 };

      int m_fieldOfView{ 45 };

      bool m_isLightAttachedToCamera{ true };
      bool m_shouldSearchDirectories{ false };
      bool m_shouldSearchFiles{ true };
      bool m_shouldShowCascadeSplitOverlay{ false };
      bool m_shouldRenderShadows{ true };
      bool m_shouldMonitorFileSystem{ true };

      JsonDocument m_fileColorMapDocument;
      JsonDocument m_preferencesDocument;

      std::experimental::filesystem::path m_preferencesPath;
      std::experimental::filesystem::path m_fileColorMapPath;

      ColorMap m_colorMap;
      PreferencesMap m_preferencesMap;

      std::wstring m_colorScheme{ L"Default" };

      VisualizationParameters m_visualizationParameters;

      Constants::FileSize::Prefix m_activeNumericPrefix{ Constants::FileSize::Prefix::BINARY };
   };
}

#endif //SETTINGSMANAGER_H
