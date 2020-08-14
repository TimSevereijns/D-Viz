#ifndef SESSIONSETTINGS_H
#define SESSIONSETTINGS_H

#include <string_view>

#include "Settings/settings.h"
#include "Model/vizBlock.h"
#include "constants.h"

#include <QObject>
#include <QVector3D>

namespace Settings
{
    class SessionSettings final : public QObject
    {
        Q_OBJECT
      public:
        /**
         * @returns The current camera speed.
         */
        double GetCameraSpeed() const;

        /**
         * @returns Gets the current mouse sensitivity.
         */
        double GetMouseSensitivity() const;

        /**
         * @return Gets the current light attentuation factor.
         */
        double GetLightAttenuationFactor() const;

        /**
         * @returns Gets the current ambient light coefficient.
         */
        double GetAmbientLightCoefficient() const;

        /**
         * @returns Gets the current material shininess.
         */
        double GetMaterialShininess() const;

        /**
         * @returns The current specular highlight color.
         */
        QVector3D GetSpecularColor() const;

        /**
         * @returns True if the primary scene light source is attached to the camera.
         */
        bool IsPrimaryLightAttachedToCamera() const;

        /**
         * @returns True if files should be searched.
         */
        bool ShouldSearchFiles() const;

        /**
         * @returns True if directories should be searched.
         */
        bool ShouldSearchDirectories() const;

        /**
         * @returns The current visualization parameters.
         */
        const Settings::VisualizationParameters& GetVisualizationParameters() const;

        /**
         * @overload
         */
        Settings::VisualizationParameters& GetVisualizationParameters();

        /**
         * @brief Sets the current visualization parameters.
         *
         * @param[in] parameters      The new visualization parameters.
         */
        Settings::VisualizationParameters&
        SetVisualizationParameters(const Settings::VisualizationParameters& parameters);

        /**
         * @brief Sets the currently active prefix to be used for file sizes; e.g., MB vs MiB.
         *
         * @param[in] prefix          The new prefix selection.
         */
        void SetActiveNumericPrefix(Constants::SizePrefix prefix);

        /**
         * @returns The currently active prefix to be used for file sizes; e.g., MB vs MiB.
         */
        Constants::SizePrefix GetActiveNumericPrefix() const;

      public slots:

        /**
         * @brief Should be called when the camera's movement speed changes.
         *
         * @param[in] speed        The new speed.
         */
        void SetCameraSpeed(double speed);

        /**
         * @brief Should be called when the mouse's movement sensitivity changes.
         *
         * @param[in] sensitivity  The new sensitivity value.
         */
        void SetMouseSensitivity(double sensitivity);

        /**
         * @brief Should be called when the scene's minimum ambient lighting changes.
         *
         * @param[in] coefficient  The new ambient lighting coefficient.
         */
        void SetAmbientLightCoefficient(double coefficient);

        /**
         * @brief Should be called when the point light's attentuation changes.
         *
         * @param[in] attenuation  The new attenuation factor.
         */
        void SetLightAttenuation(double attenuation);

        /**
         * @brief Updates the attachment of the primary light to the camera.
         *
         * @param[in] attached        Pass in true to attach light number one to the camera.
         */
        void AttachLightToCamera(bool attached);

        /**
         * @brief Handles toggling of whether regular files should be searched.
         *
         * @param[in] state           Pass in true if files should be searched for query matches.
         */
        void SearchFiles(bool state);

        /**
         * @brief Handles toggling of whether directories should be searched.
         *
         * @param[in] state           Pass in true if directories should be searched for query
         *                            matches.
         */
        void SearchDirectories(bool state);

      private:
        double m_cameraSpeed{ 0.25 };
        double m_mouseSensitivity{ 0.20 };
        double m_ambientLightCoefficient{ 0.2 };
        double m_lightAttenuationFactor{ 0.000 };
        double m_materialShininess{ 80.0 };

        bool m_isLightAttachedToCamera{ false };
        bool m_shouldSearchDirectories{ false };
        bool m_shouldSearchFiles{ true };

        Settings::VisualizationParameters m_visualizationParameters;

        Constants::SizePrefix m_activeNumericPrefix{ Constants::SizePrefix::Binary };
    };
} // namespace Settings

#endif // SESSIONSETTINGS_H
