#include "settingsManager.h"

#include <spdlog/spdlog.h>

#include <constants.h>

#ifdef Q_OS_WIN
   #undef GetObject
#endif

namespace
{
   /**
    * @brief Populates the passed in map with the flattened content of the JSON document.
    *
    * @param[in] json               The JSON document containing the file color information.
    * @param[out] map               The map that is to contain the flattened JSON data.
    */
   void PopulateColorMapFromJsonDocument(
      const Settings::JsonDocument& json,
      Settings::ColorMap& map)
   {
      if (!json.IsObject())
      {
         return;
      }

      auto encounteredError{ false };

      for (const auto& category : json.GetObject())
      {
         if (!category.value.IsObject())
         {
            encounteredError = true;
            continue;
         }

         std::unordered_map<std::wstring, QVector3D> extensionMap;

         for (const auto& extension : category.value.GetObject())
         {
            if (!extension.value.IsArray())
            {
               encounteredError = true;
               continue;
            }

            const auto colorArray = extension.value.GetArray();
            QVector3D colorVector
            {
               colorArray[0].GetFloat() / 255.0f,
               colorArray[1].GetFloat() / 255.0f,
               colorArray[2].GetFloat() / 255.0f
            };

            extensionMap.emplace(extension.name.GetString(), std::move(colorVector));
         }

         map.emplace(category.name.GetString(), std::move(extensionMap));
      }

      if (encounteredError)
      {
         const auto& log = spdlog::get(Constants::Logging::DEFAULT_LOG);
         log->error("Encountered an error converting JSON document to file color map.");
      }
   }

   /**
    * @brief Populates the passed in map with the content of the JSON document.
    *
    * @param[in] json               The JSON document containing the file color information.
    * @param[out] map               The map that is to contain the flattened JSON data.
    */
   void PopulatePreferencesMapFromJsonDocument(
      const Settings::JsonDocument& json,
      Settings::PreferencesMap& map)
   {
      if (!json.IsObject())
      {
         return;
      }

      auto encounteredError{ false };

      for (const auto& setting : json.GetObject())
      {
         if (setting.value.IsBool())
         {
            map.Emplace(setting.name.GetString(), setting.value.GetBool());
            continue;
         }

         if (setting.value.IsInt())
         {
            map.Emplace(setting.name.GetString(), setting.value.GetInt());
            continue;
         }

         if (setting.value.IsFloat())
         {
            map.Emplace(setting.name.GetString(), setting.value.GetFloat());
            continue;
         }

         if (setting.value.IsString())
         {
            map.Emplace(setting.name.GetString(), setting.value.GetString());
            continue;
         }

         if (setting.value.IsArray())
         {
            const auto colorArray = setting.value.GetArray();
            QVector3D colorVector
            {
               colorArray[0].GetFloat() / 255.0f,
               colorArray[1].GetFloat() / 255.0f,
               colorArray[2].GetFloat() / 255.0f
            };

            map.Emplace(setting.name.GetString(), std::move(colorVector));
         }
      }

      if (encounteredError)
      {
         const auto& log = spdlog::get(Constants::Logging::DEFAULT_LOG);
         log->error("Encountered unsupported type while parsing the configuration JSON file.");
      }
   }
}

namespace Settings
{
   Manager::Manager(
      const std::experimental::filesystem::path& colorFile,
      const std::experimental::filesystem::path& preferencesFile)
      :
      m_fileColorJsonDocument{ Settings::ParseJsonDocument(colorFile) },
      m_generalSettingsJsonDocument{ Settings::ParseJsonDocument(preferencesFile) }
   {
      PopulateColorMapFromJsonDocument(m_fileColorJsonDocument, m_colorMap);
      PopulatePreferencesMapFromJsonDocument(m_generalSettingsJsonDocument, m_preferencesMap);
   }

   void Manager::OnCameraSpeedChanged(double speed)
   {
      m_cameraSpeed = speed;
   }

   void Manager::OnMouseSensitivityChanged(double sensitivity)
   {
      m_mouseSensitivity = sensitivity;
   }

   void Manager::OnAmbientLightCoefficientChanged(double coefficient)
   {
      m_ambientLightCoefficient = static_cast<float>(coefficient);
   }

   void Manager::OnLightAttenuationChanged(double attenuation)
   {
      m_lightAttenuationFactor = static_cast<float>(attenuation);
   }

   void Manager::OnMaterialShininessChanged(double shininess)
   {
      m_materialShininess = static_cast<float>(shininess);
   }

   void Manager::OnAttachLightToCameraStateChanged(bool attached)
   {
      m_isLightAttachedToCamera = attached;
   }

   void Manager::OnFieldOfViewChanged(int fieldOfView)
   {
      m_fieldOfView = fieldOfView;
   }

   void Manager::OnShouldSearchFilesChanged(bool state)
   {
      m_shouldSearchFiles = state;
   }

   void Manager::OnShouldSearchDirectoriesChanged(bool state)
   {
      m_shouldSearchDirectories = state;
   }

   double Manager::GetCameraSpeed() const
   {
      return m_cameraSpeed;
   }

   void Manager::SetCameraSpeed(double speed)
   {
      m_cameraSpeed = speed;
   }

   double Manager::GetMouseSensitivity() const
   {
      return m_mouseSensitivity;
   }

   float Manager::GetLightAttentuationFactor() const
   {
      return m_lightAttenuationFactor;
   }

   float Manager::GetAmbientLightCoefficient() const
   {
      return m_ambientLightCoefficient;
   }

   float Manager::GetMaterialShininess() const
   {
      return m_materialShininess;
   }

   bool Manager::IsPrimaryLightAttachedToCamera() const
   {
      return m_isLightAttachedToCamera;
   }

   const ColorMap& Manager::GetFileColorMap() const
   {
      return m_colorMap;
   }

   const PreferencesMap& Manager::GetPreferenceMap() const
   {
      return m_preferencesMap;
   }

   const std::wstring& Manager::GetActiveColorScheme() const
   {
      return m_colorScheme;
   }

   void Manager::SetColorScheme(const std::wstring& scheme)
   {
      m_colorScheme = scheme;
   }

   const VisualizationParameters& Manager::GetVisualizationParameters() const
   {
      return m_visualizationParameters;
   }

   VisualizationParameters& Manager::GetVisualizationParameters()
   {
      return m_visualizationParameters;
   }

   VisualizationParameters& Manager::SetVisualizationParameters(
      const VisualizationParameters& parameters)
   {
      m_visualizationParameters = parameters;
      return m_visualizationParameters;
   }

   void Manager::SetActiveNumericPrefix(Constants::FileSize::Prefix prefix)
   {
      m_activeNumericPrefix = prefix;
   }

   Constants::FileSize::Prefix Manager::GetActiveNumericPrefix() const
   {
      return m_activeNumericPrefix;
   }
}
