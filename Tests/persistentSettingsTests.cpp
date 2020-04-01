#include "persistentSettingsTests.h"

#include <Settings/persistentSettings.h>
#include <constants.h>

namespace
{
    void CleanUpWorkspace()
    {
        const auto colorMapPath = Settings::PersistentSettings::GetColorJsonPath();
        if (std::filesystem::exists(colorMapPath)) {
            std::filesystem::remove(colorMapPath);
        }

        const auto preferencesMapPath = Settings::PersistentSettings::GetPreferencesJsonPath();
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

void PersistentSettingsTests::VerifyFilesAreCreatedWhenMissing() const
{
    const auto colorMapPath = Settings::PersistentSettings::GetColorJsonPath();
    const auto preferencesMapPath = Settings::PersistentSettings::GetPreferencesJsonPath();

    Settings::PersistentSettings manager{ Settings::PersistentSettings::GetColorJsonPath(),
                                          Settings::PersistentSettings::GetPreferencesJsonPath() };

    QVERIFY(std::filesystem::exists(colorMapPath) == false); //< @todo Future enhancement?
    QVERIFY(std::filesystem::exists(preferencesMapPath) == true);
}

void PersistentSettingsTests::SavingSettingsToDisk() const
{
    const auto colorMapPath = Settings::PersistentSettings::GetColorJsonPath();
    const auto preferencesMapPath = Settings::PersistentSettings::GetPreferencesJsonPath();

    Settings::PersistentSettings manager{ Settings::PersistentSettings::GetColorJsonPath(),
                                          Settings::PersistentSettings::GetPreferencesJsonPath() };

    constexpr auto shouldShowOrigin = true;
    manager.SaveSettingToDisk(Constants::Preferences::ShowOrigin, shouldShowOrigin);

    const auto jsonDocument = Settings::LoadFromDisk(preferencesMapPath);
    QVERIFY(jsonDocument.HasMember(Constants::Preferences::ShowOrigin));
    QVERIFY(jsonDocument[Constants::Preferences::ShowOrigin].IsBool());
    QVERIFY(jsonDocument[Constants::Preferences::ShowOrigin].GetBool() == shouldShowOrigin);
}

REGISTER_TEST(PersistentSettingsTests)
