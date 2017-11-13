#include "settingsManager.h"

namespace
{
   /**
    * @brief ConvertColorConfigFromJsonToMap
    *
    * @todo Add more runtime error-checking.
    *
    * @param[in] json
    * @param[out] map
    */
   void ConvertColorConfigFromJsonToMap(
      Settings::JsonDocument& json,
      std::unordered_map<std::wstring, QVector3D>& map)
   {
         if (!json.IsObject())
         {
            return;
         }

         for (const auto& category : json.GetObject())
         {
            assert(category.value.IsObject());

            for(const auto& coloring : category.value.GetObject())
            {
               assert(coloring.value.IsArray());

               const auto colorArray = coloring.value.GetArray();
               QVector3D colorVector
               {
                  colorArray[0].GetFloat() / 255.0f,
                  colorArray[1].GetFloat() / 255.0f,
                  colorArray[2].GetFloat() / 255.0f
               };

               map.emplace(coloring.name.GetString(), std::move(colorVector));
            }
         }
   }
}

namespace Settings
{
   Manager::Manager(const std::experimental::filesystem::path& colorConfigFile) :
      m_fileColorsJson{ std::move(Settings::LoadColorSettingsFromDisk(colorConfigFile)) }
   {
      ConvertColorConfigFromJsonToMap(m_fileColorsJson, m_fileColorsMap);
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

   const std::unordered_map<std::wstring, QVector3D>& Manager::GetFileColorsMap() const
   {
      return m_fileColorsMap;
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
}
