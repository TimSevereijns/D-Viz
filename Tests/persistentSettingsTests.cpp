#include "persistentSettingsTests.h"

#include <Settings/persistentSettings.h>
#include <constants.h>

namespace
{
    void CleanUpWorkspace()
    {
        const auto colorMapPath = Settings::PersistentSettings::DefaultColoringFilePath();
        if (std::filesystem::exists(colorMapPath)) {
            std::filesystem::remove(colorMapPath);
        }

        const auto preferencesMapPath = Settings::PersistentSettings::DefaultPreferencesFilePath();
        if (std::filesystem::exists(preferencesMapPath)) {
            std::filesystem::remove(preferencesMapPath);
        }
    }

    void ToggleBooleanSetting(
        void (Settings::PersistentSettings::*setter)(bool),
        bool (Settings::PersistentSettings::*getter)(void) const)
    {
        Settings::PersistentSettings manager;

        (manager.*setter)(false);
        QVERIFY((manager.*getter)() == false);

        (manager.*setter)(true);
        QVERIFY((manager.*getter)() == true);
    }

    template <typename EvaluatorType>
    void ToggleIntegralSetting(
        void (Settings::PersistentSettings::*setter)(int),
        int (Settings::PersistentSettings::*getter)(void) const, int value,
        const EvaluatorType& evaluator)
    {
        Settings::PersistentSettings manager;

        (manager.*setter)(value);
        evaluator((manager.*getter)());
    }
} // namespace

void PersistentSettingsTests::initTestCase()
{
}

void PersistentSettingsTests::cleanupTestCase()
{
    CleanUpWorkspace();
}

void PersistentSettingsTests::init()
{
    CleanUpWorkspace();
}

void PersistentSettingsTests::VerifyFilesAreCreatedWhenAbsent() const
{
    Settings::PersistentSettings manager;

    QVERIFY(std::filesystem::exists(manager.GetColoringFilePath()) == false);
    QVERIFY(std::filesystem::exists(manager.GetPreferencesFilePath()) == true);
}

void PersistentSettingsTests::SavingSettingsToDisk() const
{
    Settings::PersistentSettings manager;

    constexpr auto shouldShowOrigin = true;
    manager.RenderOrigin(shouldShowOrigin);
    manager.SaveAllPreferencesToDisk();

    const auto jsonDocument = Settings::LoadFromDisk(manager.GetPreferencesFilePath());
    QVERIFY(jsonDocument.HasMember(Constants::Preferences::ShowOrigin));
    QVERIFY(jsonDocument[Constants::Preferences::ShowOrigin].IsBool());
    QVERIFY(jsonDocument[Constants::Preferences::ShowOrigin].GetBool() == shouldShowOrigin);
}

void PersistentSettingsTests::ToggleFileMonitoring() const
{
    ToggleBooleanSetting(
        &Settings::PersistentSettings::MonitorFileSystem,
        &Settings::PersistentSettings::ShouldMonitorFileSystem);
}

void PersistentSettingsTests::ToggleShadowRendering() const
{
    ToggleBooleanSetting(
        &Settings::PersistentSettings::RenderShadows,
        &Settings::PersistentSettings::ShouldRenderShadows);
}

void PersistentSettingsTests::ToggleCascadeSplitRendering() const
{
    ToggleBooleanSetting(
        &Settings::PersistentSettings::RenderCascadeSplits,
        &Settings::PersistentSettings::ShouldRenderCascadeSplits);
}

void PersistentSettingsTests::ToggleOriginRendering() const
{
    ToggleBooleanSetting(
        &Settings::PersistentSettings::RenderOrigin,
        &Settings::PersistentSettings::ShouldRenderOrigin);
}

void PersistentSettingsTests::ToggleGridRendering() const
{
    ToggleBooleanSetting(
        &Settings::PersistentSettings::RenderGrid, &Settings::PersistentSettings::ShouldRenderGrid);
}

void PersistentSettingsTests::ToggleLightMarkerRendering() const
{
    ToggleBooleanSetting(
        &Settings::PersistentSettings::RenderLightMarkers,
        &Settings::PersistentSettings::ShouldRenderLightMarkers);
}

void PersistentSettingsTests::ToggleFrustaRendering() const
{
    ToggleBooleanSetting(
        &Settings::PersistentSettings::RenderFrusta,
        &Settings::PersistentSettings::ShouldRenderFrusta);
}

void PersistentSettingsTests::ModifyShadowMapCascadeCount() const
{
    constexpr auto desired = 2;

    ToggleIntegralSetting(
        &Settings::PersistentSettings::SetShadowMapCascadeCount,
        &Settings::PersistentSettings::GetShadowMapCascadeCount, desired,
        [&](auto value) { QVERIFY(value == desired); });
}

void PersistentSettingsTests::VerifyClampingOfShadowMapCascadeCount() const
{
    constexpr auto desired = 20;
    constexpr auto max = 4;

    ToggleIntegralSetting(
        &Settings::PersistentSettings::SetShadowMapCascadeCount,
        &Settings::PersistentSettings::GetShadowMapCascadeCount, desired,
        [&](auto value) { QVERIFY(value == max); });
}

void PersistentSettingsTests::ModifyShadowMapQuality() const
{
    constexpr auto desired = 2;

    ToggleIntegralSetting(
        &Settings::PersistentSettings::SetShadowMapQuality,
        &Settings::PersistentSettings::GetShadowMapQuality, desired,
        [&](auto value) { QVERIFY(value == desired); });
}

void PersistentSettingsTests::VerifyClampingOfShadowMapQuality() const
{
    constexpr auto desired = 20;
    constexpr auto max = 4;

    ToggleIntegralSetting(
        &Settings::PersistentSettings::SetShadowMapQuality,
        &Settings::PersistentSettings::GetShadowMapQuality, desired,
        [&](auto value) { QVERIFY(value == max); });
}

REGISTER_TEST(PersistentSettingsTests)
