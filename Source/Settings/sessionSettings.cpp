#include "Settings/sessionSettings.h"

namespace Settings
{
    double SessionSettings::GetCameraSpeed() const
    {
        return m_cameraSpeed;
    }

    void SessionSettings::SetCameraSpeed(double speed)
    {
        m_cameraSpeed = speed;
    }

    double SessionSettings::GetMouseSensitivity() const
    {
        return m_mouseSensitivity;
    }

    double SessionSettings::GetLightAttenuationFactor() const
    {
        return m_lightAttenuationFactor;
    }

    double SessionSettings::GetAmbientLightCoefficient() const
    {
        return m_ambientLightCoefficient;
    }

    double SessionSettings::GetMaterialShininess() const
    {
        return m_materialShininess;
    }

    QVector3D SessionSettings::GetSpecularColor() const
    {
        return Constants::Colors::White;
    }

    bool SessionSettings::IsPrimaryLightAttachedToCamera() const
    {
        return m_isLightAttachedToCamera;
    }

    void SessionSettings::SetMouseSensitivity(double sensitivity)
    {
        m_mouseSensitivity = sensitivity;
    }

    void SessionSettings::SetAmbientLightCoefficient(double coefficient)
    {
        m_ambientLightCoefficient = coefficient;
    }

    void SessionSettings::SetLightAttenuation(double attenuation)
    {
        m_lightAttenuationFactor = attenuation;
    }

    void SessionSettings::AttachLightToCamera(bool attached)
    {
        m_isLightAttachedToCamera = attached;
    }

    void SessionSettings::SearchFiles(bool state)
    {
        m_shouldSearchFiles = state;
    }

    void SessionSettings::SearchDirectories(bool state)
    {
        m_shouldSearchDirectories = state;
    }

    bool SessionSettings::ShouldSearchFiles() const
    {
        return m_shouldSearchFiles;
    }

    bool SessionSettings::ShouldSearchDirectories() const
    {
        return m_shouldSearchDirectories;
    }

    const Settings::VisualizationParameters& SessionSettings::GetVisualizationParameters() const
    {
        return m_visualizationParameters;
    }

    Settings::VisualizationParameters& SessionSettings::GetVisualizationParameters()
    {
        return m_visualizationParameters;
    }

    Settings::VisualizationParameters&
    SessionSettings::SetVisualizationParameters(const Settings::VisualizationParameters& parameters)
    {
        m_visualizationParameters = parameters;
        return m_visualizationParameters;
    }

    void SessionSettings::SetActiveNumericPrefix(Constants::SizePrefix prefix)
    {
        m_activeNumericPrefix = prefix;
    }

    Constants::SizePrefix SessionSettings::GetActiveNumericPrefix() const
    {
        return m_activeNumericPrefix;
    }
} // namespace Settings
