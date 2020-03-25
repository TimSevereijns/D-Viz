#include "Settings/persistentSettings.h"
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
    PersistentSettings::PersistentSettings(
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

    JsonDocument PersistentSettings::CreatePreferencesDocument()
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

    const ColorMap& PersistentSettings::GetFileColorMap() const
    {
        return m_colorMap;
    }

    const PreferencesMap& PersistentSettings::GetPreferenceMap() const
    {
        return m_preferencesMap;
    }

    const std::wstring& PersistentSettings::GetActiveColorScheme() const
    {
        return m_colorScheme;
    }

    void PersistentSettings::SetColorScheme(const std::wstring& scheme)
    {
        m_colorScheme = scheme;
    }

    bool PersistentSettings::ShouldShowCascadeSplits() const
    {
        return m_showCascadeSplits;
    }

    void PersistentSettings::SetShowCascadeSplits(bool isEnabled)
    {
        m_showCascadeSplits = isEnabled;
    }

    bool PersistentSettings::ShouldRenderShadows() const
    {
        return m_shouldShowShadows;
    }

    void PersistentSettings::SetShowShadows(bool isEnabled)
    {
        m_shouldShowShadows = isEnabled;
    }

    bool PersistentSettings::ShouldMonitorFileSystem() const
    {
        return m_shouldMonitorFileSystem;
    }

    void PersistentSettings::OnMonitoringOptionToggled(bool isEnabled)
    {
        m_shouldMonitorFileSystem = isEnabled;
    }

    std::optional<QVector3D>
    PersistentSettings::DetermineColorFromExtension(const Tree<VizBlock>::Node& node) const
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
} // namespace Settings
