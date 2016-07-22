#include "optionsManager.h"

void OptionsManager::OnCameraMovementSpeedChanged(const double newSpeed)
{
   m_cameraMovementSpeed = newSpeed;
}

void OptionsManager::OnMouseSensitivityChanged(const double newSensitivity)
{
   m_mouseSensitivity = newSensitivity;
}

void OptionsManager::OnAmbientCoefficientChanged(const double newCoefficient)
{
   m_ambientCoefficient = static_cast<float>(newCoefficient);
}

void OptionsManager::OnAttenuationChanged(const double newAttenuation)
{
   m_lightAttenuationFactor = static_cast<float>(newAttenuation);
}

void OptionsManager::OnShininessChanged(const double newShininess)
{
   m_materialShininess = static_cast<float>(newShininess);
}

void OptionsManager::OnUseXBoxControllerStateChanged(const bool useController)
{
   m_useXBoxController = useController;
}

void OptionsManager::OnAttachLightToCameraStateChanged(const bool attached)
{
   m_isLightAttachedToCamera = attached;
}

void OptionsManager::OnFieldOfViewChanged(int fieldOfView)
{
   m_fieldOfView = fieldOfView;
}
