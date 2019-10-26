#include "Settings/settingsManager.h"
#include <Visualizations/vizBlock.h>
#include <constants.h>

#include <spdlog/spdlog.h>

#ifdef Q_OS_WIN
#undef GetObject
#endif // Q_OS_WIN

namespace
{
    /**
     * @brief Populates the passed in map with the flattened content of the JSON document.
     *
     * @param[in] json               The JSON document containing the file color information.
     * @param[out] map               The map that is to contain the flattened JSON data.
     */
    void
    PopulateColorMapFromJsonDocument(const Settings::JsonDocument& json, Settings::ColorMap& map)
    {
        if (!json.IsObject()) {
            return;
        }

        auto encounteredError{ false };

        for (const auto& category : json.GetObject()) {
            if (!category.value.IsObject()) {
                encounteredError = true;
                continue;
            }

            std::unordered_map<std::wstring, QVector3D> extensionMap;

            for (const auto& extension : category.value.GetObject()) {
                if (!extension.value.IsArray()) {
                    encounteredError = true;
                    continue;
                }

                const auto colorArray = extension.value.GetArray();
                QVector3D colorVector{ colorArray[0].GetFloat() / 255.0f,
                                       colorArray[1].GetFloat() / 255.0f,
                                       colorArray[2].GetFloat() / 255.0f };

                extensionMap.emplace(extension.name.GetString(), colorVector);
            }

            map.emplace(category.name.GetString(), std::move(extensionMap));
        }

        if (encounteredError) {
            const auto& log = spdlog::get(Constants::Logging::DefaultLog);
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
        const Settings::JsonDocument& json, Settings::PreferencesMap& map)
    {
        if (!json.IsObject()) {
            return;
        }

        auto encounteredError{ false };

        for (const auto& setting : json.GetObject()) {
            if (setting.value.IsBool()) {
                map.Emplace(setting.name.GetString(), setting.value.GetBool());
                continue;
            }

            if (setting.value.IsInt()) {
                map.Emplace(setting.name.GetString(), setting.value.GetInt());
                continue;
            }

            if (setting.value.IsFloat()) {
                map.Emplace(setting.name.GetString(), setting.value.GetFloat());
                continue;
            }

            if (setting.value.IsString()) {
                map.Emplace(setting.name.GetString(), setting.value.GetString());
                continue;
            }

            if (setting.value.IsArray()) {
                const auto colorArray = setting.value.GetArray();
                QVector3D colorVector{ colorArray[0].GetFloat() / 255.0f,
                                       colorArray[1].GetFloat() / 255.0f,
                                       colorArray[2].GetFloat() / 255.0f };

                map.Emplace(setting.name.GetString(), colorVector);
            }
        }

        if (encounteredError) {
            const auto& log = spdlog::get(Constants::Logging::DefaultLog);
            log->error("Encountered unsupported type while parsing the configuration JSON file.");
        }
    }
} // namespace

namespace Settings
{
    Manager::Manager(
        const std::filesystem::path& colorFile, const std::filesystem::path& preferencesFile)
        : m_preferencesPath{ preferencesFile }, m_fileColorMapPath{ colorFile }
    {
        if (!std::filesystem::exists(m_preferencesPath)) {
            m_preferencesDocument = CreatePreferencesDocument();
        } else {
            m_preferencesDocument = LoadFromDisk(m_preferencesPath);
        }

        PopulateColorMapFromJsonDocument(m_fileColorMapDocument, m_colorMap);
        PopulatePreferencesMapFromJsonDocument(m_preferencesDocument, m_preferencesMap);
    }

    JsonDocument Manager::CreatePreferencesDocument()
    {
        JsonDocument document;
        document.SetObject();

        auto& allocator = document.GetAllocator();

        document.AddMember(Constants::Preferences::ShowGrid, true, allocator);
        document.AddMember(Constants::Preferences::ShowOrigin, false, allocator);
        document.AddMember(Constants::Preferences::ShowFrusta, false, allocator);
        document.AddMember(Constants::Preferences::ShowLights, false, allocator);
        document.AddMember(Constants::Preferences::ShowShadows, true, allocator);
        document.AddMember(Constants::Preferences::ShowCascadeSplits, false, allocator);
        document.AddMember(Constants::Preferences::ShadowMapQuality, 4, allocator);
        document.AddMember(Constants::Preferences::ShowDebuggingMenu, false, allocator);

        SaveToDisk(document, m_preferencesPath);

        return document;
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
        m_ambientLightCoefficient = coefficient;
    }

    void Manager::OnLightAttenuationChanged(double attenuation)
    {
        m_lightAttenuationFactor = attenuation;
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

    void Manager::OnMonitoringOptionToggled(bool isEnabled)
    {
        m_shouldMonitorFileSystem = isEnabled;
    }

    bool Manager::ShouldBlockBeProcessed(const VizBlock& block)
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

    std::optional<QVector3D>
    Manager::DetermineColorFromExtension(const Tree<VizBlock>::Node& node) const
    {
        const auto categoryItr = m_colorMap.find(m_colorScheme);
        if (categoryItr == std::end(m_colorMap)) {
            return std::nullopt;
        }

        const auto extensionItr = categoryItr->second.find(node->file.extension);
        if (extensionItr == std::end(categoryItr->second)) {
            return std::nullopt;
        }

        return extensionItr->second;
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

    double Manager::GetLightAttentuationFactor() const
    {
        return m_lightAttenuationFactor;
    }

    double Manager::GetAmbientLightCoefficient() const
    {
        return m_ambientLightCoefficient;
    }

    double Manager::GetMaterialShininess() const
    {
        return m_materialShininess;
    }

    QVector3D Manager::GetSpecularColor() const
    {
        return Constants::Colors::White;
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

    VisualizationParameters&
    Manager::SetVisualizationParameters(const VisualizationParameters& parameters)
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

    void Manager::SetShowCascadeSplits(bool isEnabled)
    {
        m_showCascadeSplits = isEnabled;
    }

    bool Manager::ShouldShowCascadeSplits() const
    {
        return m_showCascadeSplits;
    }

    void Manager::SetShowShadows(bool isEnabled)
    {
        m_shouldShowShadows = isEnabled;
    }

    bool Manager::ShouldRenderShadows() const
    {
        return m_shouldShowShadows;
    }

    bool Manager::ShouldMonitorFileSystem() const
    {
        return m_shouldMonitorFileSystem;
    }
} // namespace Settings
