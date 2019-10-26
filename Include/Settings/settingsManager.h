#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include "constants.h"
#include "preferencesMap.hpp"
#include "settings.h"

#include <optional>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <variant>

#include <Tree/Tree.hpp>

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
            const std::filesystem::path& colorFile, const std::filesystem::path& preferencesFile);

        /**
         * @returns The current camera speed.
         */
        double GetCameraSpeed() const;

        /**
         * @brief Sets the current camera speed.
         *
         * @param[in] speed           The new speed.
         */
        void SetCameraSpeed(double speed);

        /**
         * @returns Gets the current mouse sensitivity.
         */
        double GetMouseSensitivity() const;

        /**
         * @return Gets the current light attentuation factor.
         */
        double GetLightAttentuationFactor() const;

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
         * @returns The map that associates colors with file extensions.
         */
        const ColorMap& GetFileColorMap() const;

        /**
         * @returns The preference map.
         */
        const PreferencesMap& GetPreferenceMap() const;

        /**
         * @returns The currently active file extension coloring scheme.
         */
        const std::wstring& GetActiveColorScheme() const;

        /**
         * @brief Sets the current color scheme.
         */
        void SetColorScheme(const std::wstring& scheme);

        /**
         * @returns The current visualization parameters.
         */
        const VisualizationParameters& GetVisualizationParameters() const;

        /**
         * @overload
         */
        VisualizationParameters& GetVisualizationParameters();

        /**
         * @brief Sets the current visualization parameters.
         *
         * @param[in] parameters      The new visualization parameters.
         */
        VisualizationParameters&
        SetVisualizationParameters(const VisualizationParameters& parameters);

        /**
         * @brief Sets the currently active prefix to be used for file sizes; e.g., MB vs MiB.
         *
         * @param[in] prefix          The new prefix selection.
         */
        void SetActiveNumericPrefix(Constants::FileSize::Prefix prefix);

        /**
         * @returns The currently active prefix to be used for file sizes; e.g., MB vs MiB.
         */
        Constants::FileSize::Prefix GetActiveNumericPrefix() const;

        /**
         * @returns True if shadow cascade splits should be visualized.
         */
        bool ShouldShowCascadeSplits() const;

        /**
         * @brief Passing in `true` will render the cascade splits overlay.
         */
        void SetShowCascadeSplits(bool isEnabled);

        /**
         * @returns True if shadow rendering is enabled.
         */
        bool ShouldRenderShadows() const;

        /**
         * @brief Passing in `true` will enable the rendering of shadows.
         */
        void SetShowShadows(bool isEnabled);

        /**
         * @returns True if the file system monitor is enabled;
         */
        bool ShouldMonitorFileSystem() const;

        /**
         * @brief ShouldNodeBeIgnored
         * @return
         */
        bool ShouldBlockBeProcessed(const VizBlock& block);

        /**
         * @brief Determines the appropriate color for the file based on the user-configurable color
         * set in the color.json file.
         *
         * @param[in] node               The node whose color needs to be restored.
         *
         * @returns The appropriate color found in the color map.
         */
        std::optional<QVector3D>
        DetermineColorFromExtension(const Tree<VizBlock>::Node& node) const;

        /**
         * @brief Saves preferences to a JSON file on disk.
         *
         * @todo Make private (will have to move some other functionality around as well).
         *
         * @param[in] property        The name of the property to be saved or modified.
         * @param[in] value           The value to be associated with the property name.
         *
         * @returns True if the save action succeeded.
         */
        template <typename PropertyValueType>
        bool SavePreferenceChangeToDisk(std::wstring_view property, const PropertyValueType& value)
        {
            if (m_preferencesDocument.HasMember(property.data())) {
                m_preferencesDocument[property.data()] = value;
            } else {
                auto& allocator = m_preferencesDocument.GetAllocator();

                rapidjson::GenericValue<rapidjson::UTF16<wchar_t>> key{ property.data(),
                                                                        allocator };
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
         * @brief Handles toggling of whether regular files should be searched.
         *
         * @param[in] state           Pass in true if files should be searched for query matches.
         */
        void OnShouldSearchFilesChanged(bool state);

        /**
         * @brief Handles toggling of whether directories should be searched.
         *
         * @param[in] state           Pass in true if directories should be searched for query
         *                            matches.
         */
        void OnShouldSearchDirectoriesChanged(bool state);

        /**
         * @brief Handles toggling of whether the file system should be monitored for changes.
         *
         * @param[in] isEnabled       Pass in true to enable monitoring.
         */
        void OnMonitoringOptionToggled(bool isEnabled);

      private:
        JsonDocument CreatePreferencesDocument();

        double m_cameraSpeed{ 0.25 };
        double m_mouseSensitivity{ 0.20 };

        double m_ambientLightCoefficient{ 0.2 };
        double m_lightAttenuationFactor{ 0.002 };
        double m_materialShininess{ 80.0 };

        int m_fieldOfView{ 45 };

        bool m_isLightAttachedToCamera{ true };
        bool m_shouldSearchDirectories{ false };
        bool m_shouldSearchFiles{ true };
        bool m_showCascadeSplits{ false };
        bool m_shouldShowShadows{ true };
        bool m_shouldMonitorFileSystem{ true };

        JsonDocument m_fileColorMapDocument;
        JsonDocument m_preferencesDocument;

        std::filesystem::path m_preferencesPath;
        std::filesystem::path m_fileColorMapPath;

        ColorMap m_colorMap;
        PreferencesMap m_preferencesMap;

        std::wstring m_colorScheme{ L"Default" };

        VisualizationParameters m_visualizationParameters;

        Constants::FileSize::Prefix m_activeNumericPrefix{ Constants::FileSize::Prefix::BINARY };
    };
} // namespace Settings

#endif // SETTINGSMANAGER_H
