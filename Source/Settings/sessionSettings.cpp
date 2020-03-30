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

    void SessionSettings::OnCameraSpeedChanged(double speed)
    {
        m_cameraSpeed = speed;
    }

    void SessionSettings::OnMouseSensitivityChanged(double sensitivity)
    {
        m_mouseSensitivity = sensitivity;
    }

    void SessionSettings::OnAmbientLightCoefficientChanged(double coefficient)
    {
        m_ambientLightCoefficient = coefficient;
    }

    void SessionSettings::OnLightAttenuationChanged(double attenuation)
    {
        m_lightAttenuationFactor = attenuation;
    }

    void SessionSettings::OnAttachLightToCameraStateChanged(bool attached)
    {
        m_isLightAttachedToCamera = attached;
    }

    void SessionSettings::OnFieldOfViewChanged(int fieldOfView)
    {
        m_fieldOfView = fieldOfView;
    }

    void SessionSettings::OnShouldSearchFilesChanged(bool state)
    {
        m_shouldSearchFiles = state;
    }

    void SessionSettings::OnShouldSearchDirectoriesChanged(bool state)
    {
        m_shouldSearchDirectories = state;
    }

    bool SessionSettings::IsBlockVisible(const VizBlock& block)
    {
        if (block.file.size < m_visualizationParameters.minimumFileSize) {
            return false;
        }

        if (block.file.type != FileType::DIRECTORY &&
            m_visualizationParameters.onlyShowDirectories) {
            return false;
        }

        return true;
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

    void SessionSettings::SetActiveNumericPrefix(Constants::FileSize::Prefix prefix)
    {
        m_activeNumericPrefix = prefix;
    }

    Constants::FileSize::Prefix SessionSettings::GetActiveNumericPrefix() const
    {
        return m_activeNumericPrefix;
    }
} // namespace Settings
