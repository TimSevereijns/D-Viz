#include "Settings/sessionSettings.h"
#include "Settings/visualizationOptions.h"

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

    void SessionSettings::UseRegexSearch(bool state)
    {
        m_shouldUseRegex = state;
    }

    bool SessionSettings::ShouldSearchFiles() const
    {
        return m_shouldSearchFiles;
    }

    bool SessionSettings::ShouldSearchDirectories() const
    {
        return m_shouldSearchDirectories;
    }

    bool SessionSettings::ShouldUseRegex() const
    {
        return m_shouldUseRegex;
    }

    const Settings::VisualizationOptions& SessionSettings::GetVisualizationOptions() const
    {
        return m_visualizationOptions;
    }

    Settings::VisualizationOptions& SessionSettings::GetVisualizationOptions()
    {
        return m_visualizationOptions;
    }

    Settings::VisualizationOptions&
    SessionSettings::SetVisualizationOptions(const Settings::VisualizationOptions& options)
    {
        m_visualizationOptions = options;
        return m_visualizationOptions;
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
