#include "Settings/persistentSettings.h"

#include <Model/vizBlock.h>
#include <constants.h>

#include <algorithm>

#include <gsl/assert>
#include <spdlog/spdlog.h>

namespace
{
    template <typename DataType>
    auto GetValueOrDefault(
        const Settings::JsonDocument& document, std::string_view preference, DataType defaultValue)
    {
        const auto itr = document.FindMember(preference.data());

        if (itr == document.MemberEnd()) {
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
    void SaveValue(Settings::JsonDocument& document, std::string_view preference, DataType value)
    {
        const auto itr = document.FindMember(preference.data());

        if (itr == document.MemberEnd()) {
            auto& allocator = document.GetAllocator();
            rapidjson::Value key{ preference.data(), allocator };
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
    PersistentSettings::PersistentSettings(const std::filesystem::path& preferencesFile)
        : m_preferencesPath{ preferencesFile }
    {
        if (!std::filesystem::exists(m_preferencesPath)) {
            m_preferencesDocument = CreatePreferencesDocument();
        } else {
            m_preferencesDocument = LoadFromDisk(m_preferencesPath);
        }
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
        constexpr auto defaultValue = 8;
        const auto quality = GetValueOrDefault(
            m_preferencesDocument, Constants::Preferences::ShadowMapQuality, defaultValue);

        return std::clamp(quality, 1, 8);
    }

    void PersistentSettings::SetShadowMapQuality(int quality)
    {
        SaveValue(
            m_preferencesDocument, Constants::Preferences::ShadowMapQuality,
            std::clamp(quality, 1, 8));
    }

    bool PersistentSettings::ShouldShowDebuggingMenu() const
    {
        constexpr auto defaultValue = false;
        return GetValueOrDefault(
            m_preferencesDocument, Constants::Preferences::ShowDebuggingMenu, defaultValue);
    }

    void PersistentSettings::UseDarkMode(bool isEnabled)
    {
        SaveValue(m_preferencesDocument, Constants::Preferences::UseDarkMode, isEnabled);
    }

    bool PersistentSettings::ShouldUseDarkMode() const
    {
        constexpr auto defaultValue = false;
        return GetValueOrDefault(
            m_preferencesDocument, Constants::Preferences::UseDarkMode, defaultValue);
    }

    bool PersistentSettings::SaveAllPreferencesToDisk()
    {
        return SaveToDisk(m_preferencesDocument, m_preferencesPath);
    }

    const std::filesystem::path& PersistentSettings::GetPreferencesFilePath() const
    {
        return m_preferencesPath;
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
        document.AddMember(Constants::Preferences::UseDarkMode, false, allocator);

        SaveToDisk(document, m_preferencesPath);

        return document;
    }
} // namespace Settings
