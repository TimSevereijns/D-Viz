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
    manager.SaveSettingToDisk(Constants::Preferences::ShowOrigin, shouldShowOrigin);

    const auto jsonDocument = Settings::LoadFromDisk(manager.GetPreferencesFilePath());
    QVERIFY(jsonDocument.HasMember(Constants::Preferences::ShowOrigin));
    QVERIFY(jsonDocument[Constants::Preferences::ShowOrigin].IsBool());
    QVERIFY(jsonDocument[Constants::Preferences::ShowOrigin].GetBool() == shouldShowOrigin);
}

void PersistentSettingsTests::ToggleFileMonitoring() const
{
    Settings::PersistentSettings manager;

    manager.MonitorFileSystem(false);
    QVERIFY(manager.ShouldMonitorFileSystem() == false);

    manager.MonitorFileSystem(true);
    QVERIFY(manager.ShouldMonitorFileSystem() == true);
}

void PersistentSettingsTests::ToggleShadowRendering() const
{
    Settings::PersistentSettings manager;

    manager.RenderShadows(false);
    QVERIFY(manager.ShouldRenderShadows() == false);

    manager.RenderShadows(true);
    QVERIFY(manager.ShouldRenderShadows() == true);
}

void PersistentSettingsTests::ToggleCascadeSplitRendering() const
{
    Settings::PersistentSettings manager;

    manager.RenderCascadeSplits(false);
    QVERIFY(manager.ShouldRenderCascadeSplits() == false);

    manager.RenderCascadeSplits(true);
    QVERIFY(manager.ShouldRenderCascadeSplits() == true);
}

REGISTER_TEST(PersistentSettingsTests)
