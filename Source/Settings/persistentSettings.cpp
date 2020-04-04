#include "Settings/persistentSettings.h"

#include <Visualizations/vizBlock.h>
#include <constants.h>

#include <algorithm>

#include <gsl/gsl_assert>
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

    template <typename DataType>
    auto GetValueOrDefault(
        const Settings::JsonDocument& document, std::wstring_view preference, DataType defaultValue)
    {
        const auto itr = document.FindMember(preference.data());

        if (itr == document.MemberEnd()) {
            if constexpr (std::is_same_v<DataType, bool>) {
                if (!itr->value.IsBool()) {
                    return defaultValue;
                }
            } else if constexpr (std::is_same_v<DataType, int>) {
                if (!itr->value.IsInt()) {
                    return defaultValue;
                }
            }

            return defaultValue;
        }

        if constexpr (std::is_same_v<DataType, bool>) {
            return itr->value.GetBool();
        } else if constexpr (std::is_same_v<DataType, int>) {
            return itr->value.GetInt();
        } else {
            GSL_ASSUME(false);
        }
    }

    template <typename DataType>
    void SaveValue(Settings::JsonDocument& document, std::wstring_view preference, DataType value)
    {
        const auto itr = document.FindMember(preference.data());

        if (itr == document.MemberEnd()) {
            auto& allocator = document.GetAllocator();

            rapidjson::GenericValue<rapidjson::UTF16<>> key{ preference.data(), allocator };
            document.AddMember(key.Move(), value, allocator);

            return;
        }

        if constexpr (std::is_same_v<DataType, bool>) {
            itr->value.SetBool(value);
        } else if constexpr (std::is_same_v<DataType, int>) {
            itr->value.SetInt(value);
        } else {
            GSL_ASSUME(false);
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
    }

    const ColorMap& PersistentSettings::GetFileColorMap() const
    {
        return m_colorMap;
    }

    const std::wstring& PersistentSettings::GetActiveColorScheme() const
    {
        return m_colorScheme;
    }

    void PersistentSettings::SetColorScheme(const std::wstring& scheme)
    {
        m_colorScheme = scheme;
    }

    bool PersistentSettings::ShouldRenderCascadeSplits() const
    {
        constexpr auto defaultValue = false;

        return GetValueOrDefault(
            m_preferencesDocument, Constants::Preferences::ShowCascadeSplits, defaultValue);
    }

    void PersistentSettings::RenderCascadeSplits(bool isEnabled)
    {
        SaveValue(m_preferencesDocument, Constants::Preferences::ShowCascadeSplits, isEnabled);
    }

    bool PersistentSettings::ShouldRenderShadows() const
    {
        constexpr auto defaultValue = true;

        return GetValueOrDefault(
            m_preferencesDocument, Constants::Preferences::ShowShadows, defaultValue);
    }

    void PersistentSettings::RenderShadows(bool isEnabled)
    {
        SaveValue(m_preferencesDocument, Constants::Preferences::ShowShadows, isEnabled);
    }

    bool PersistentSettings::ShouldMonitorFileSystem() const
    {
        constexpr auto defaultValue = false;

        return GetValueOrDefault(
            m_preferencesDocument, Constants::Preferences::MonitorFileSystem, defaultValue);
    }

    void PersistentSettings::MonitorFileSystem(bool isEnabled)
    {
        SaveValue(m_preferencesDocument, Constants::Preferences::MonitorFileSystem, isEnabled);
    }

    bool PersistentSettings::ShouldRenderOrigin() const
    {
        constexpr auto defaultValue = true;

        return GetValueOrDefault(
            m_preferencesDocument, Constants::Preferences::ShowOrigin, defaultValue);
    }

    void PersistentSettings::RenderOrigin(bool isEnabled)
    {
        SaveValue(m_preferencesDocument, Constants::Preferences::ShowOrigin, isEnabled);
    }

    bool PersistentSettings::ShouldRenderGrid() const
    {
        constexpr auto defaultValue = true;

        return GetValueOrDefault(
            m_preferencesDocument, Constants::Preferences::ShowGrid, defaultValue);
    }

    void PersistentSettings::RenderGrid(bool isEnabled)
    {
        SaveValue(m_preferencesDocument, Constants::Preferences::ShowGrid, isEnabled);
    }

    bool PersistentSettings::ShouldRenderLightMarkers() const
    {
        constexpr auto defaultValue = false;

        return GetValueOrDefault(
            m_preferencesDocument, Constants::Preferences::ShowLightMarkers, defaultValue);
    }

    void PersistentSettings::RenderLightMarkers(bool isEnabled)
    {
        SaveValue(m_preferencesDocument, Constants::Preferences::ShowLightMarkers, isEnabled);
    }

    bool PersistentSettings::ShouldRenderFrusta() const
    {
        constexpr auto defaultValue = false;

        return GetValueOrDefault(
            m_preferencesDocument, Constants::Preferences::ShowFrusta, defaultValue);
    }

    void PersistentSettings::RenderFrusta(bool isEnabled)
    {
        SaveValue(m_preferencesDocument, Constants::Preferences::ShowFrusta, isEnabled);
    }

    int PersistentSettings::GetShadowMapCascadeCount() const
    {
        constexpr auto defaultValue = 4;

        const auto count = GetValueOrDefault(
            m_preferencesDocument, Constants::Preferences::ShadowMapCascadeCount, defaultValue);

        return std::clamp(count, 1, 4);
    }

    void PersistentSettings::SetShadowMapCascadeCount(int count)
    {
        SaveValue(
            m_preferencesDocument, Constants::Preferences::ShadowMapCascadeCount,
            std::clamp(count, 1, 4));
    }

    int PersistentSettings::GetShadowMapQuality() const
    {
        constexpr auto defaultValue = 4;

        const auto quality = GetValueOrDefault(
            m_preferencesDocument, Constants::Preferences::ShadowMapQuality, defaultValue);

        return std::clamp(quality, 1, 4);
    }

    void PersistentSettings::SetShadowMapQuality(int quality)
    {
        SaveValue(
            m_preferencesDocument, Constants::Preferences::ShadowMapQuality,
            std::clamp(quality, 1, 4));
    }

    bool PersistentSettings::ShouldShowDebuggingMenu() const
    {
        constexpr auto defaultValue = false;

        return GetValueOrDefault(
            m_preferencesDocument, Constants::Preferences::ShowDebuggingMenu, defaultValue);
    }

    bool PersistentSettings::SaveAllPreferencesToDisk()
    {
        return SaveToDisk(m_preferencesDocument, m_preferencesPath);
    }

    const std::filesystem::path& PersistentSettings::GetColoringFilePath() const
    {
        return m_fileColorMapPath;
    }

    const std::filesystem::path& PersistentSettings::GetPreferencesFilePath() const
    {
        return m_preferencesPath;
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

    JsonDocument PersistentSettings::CreatePreferencesDocument()
    {
        JsonDocument document;
        document.SetObject();

        auto& allocator = document.GetAllocator();

        document.AddMember(Constants::Preferences::ShowGrid, true, allocator);
        document.AddMember(Constants::Preferences::ShowOrigin, false, allocator);
        document.AddMember(Constants::Preferences::ShowFrusta, false, allocator);
        document.AddMember(Constants::Preferences::ShowLightMarkers, false, allocator);
        document.AddMember(Constants::Preferences::ShowShadows, true, allocator);
        document.AddMember(Constants::Preferences::ShowCascadeSplits, false, allocator);
        document.AddMember(Constants::Preferences::ShadowMapQuality, 4, allocator);
        document.AddMember(Constants::Preferences::ShowDebuggingMenu, false, allocator);
        document.AddMember(Constants::Preferences::MonitorFileSystem, false, allocator);

        SaveToDisk(document, m_preferencesPath);

        return document;
    }
} // namespace Settings
