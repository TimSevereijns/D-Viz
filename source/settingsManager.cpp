#include "SettingsManager.h"

namespace Settings
{
   Manager::Manager(const std::experimental::filesystem::path& colorConfigFile) :
      m_fileColors{ std::move(Settings::LoadColorSettingsFromDisk(colorConfigFile)) }
   {
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

   const JsonDocument& Manager::GetFileColors() const
   {
      return m_fileColors;
   }

   const VisualizationParameters& Manager::GetVisualizationParameters() const
   {
      return m_visualizationParameters;
   }

   void Manager::SetVisualizationParameters(const VisualizationParameters& parameters)
   {
      m_visualizationParameters = parameters;
   }
}
