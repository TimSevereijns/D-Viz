#include "optionsManager.h"

OptionsManager::OptionsManager()
   : m_cameraMovementSpeed(0.25),
     m_mouseSensitivity(0.25),
     m_ambientCoefficient(0.005),
     m_lightAttenuationFactor(0.05),
     m_materialShininess(80.0),
     m_fieldOfView(45),
     m_redLightComponent(1.0f),
     m_greenLightComponent(1.0f),
     m_blueLightComponent(1.0f),
     m_isXboxControllerConnected(false),
     m_useXBoxController(false),
     m_isLightAttachedToCamera(true)
{
}

OptionsManager::~OptionsManager()
{

}

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

void OptionsManager::OnRedLightComponentChanged(const int value)
{
   m_redLightComponent = static_cast<float>(value) / 100.0;
}

void OptionsManager::OnGreenLightComponentChanged(const int value)
{
   m_greenLightComponent = static_cast<float>(value) / 100.0;
}

void OptionsManager::OnBlueLightComponentChanged(const int value)
{
   m_blueLightComponent = static_cast<float>(value) / 100.0;
}

