#include "sessionSettingsTests.h"

#include <Settings/sessionSettings.h>

void SessionSettingsTests::initTestCase()
{
}

void SessionSettingsTests::cleanupTestCase()
{
}

void SessionSettingsTests::init()
{
}

void SessionSettingsTests::ModifyCameraSpeed() const
{
    constexpr auto desiredSpeed = 50;

    Settings::SessionSettings manager;
    manager.SetCameraSpeed(desiredSpeed);

    QCOMPARE(manager.GetCameraSpeed(), desiredSpeed);
}

void SessionSettingsTests::ModifyMouseSensitivity() const
{
    constexpr auto desiredSensitivity = 50;

    Settings::SessionSettings manager;
    manager.SetMouseSensitivity(desiredSensitivity);

    QCOMPARE(manager.GetMouseSensitivity(), desiredSensitivity);
}

void SessionSettingsTests::ModifyLightAttenuationFactor() const
{
    constexpr auto desiredFactor = 50;

    Settings::SessionSettings manager;
    manager.SetLightAttenuation(desiredFactor);

    QCOMPARE(manager.GetLightAttenuationFactor(), desiredFactor);
}

void SessionSettingsTests::ModifyAmbientLightCoefficient() const
{
    constexpr auto desiredCoefficient = 50;

    Settings::SessionSettings manager;
    manager.SetAmbientLightCoefficient(desiredCoefficient);

    QCOMPARE(manager.GetAmbientLightCoefficient(), desiredCoefficient);
}

void SessionSettingsTests::ModifyPrimaryLightAttachmentToCamera() const
{
    constexpr auto desiredAttachment = true;

    Settings::SessionSettings manager;
    manager.AttachLightToCamera(desiredAttachment);

    QCOMPARE(manager.IsPrimaryLightAttachedToCamera(), desiredAttachment);
}

void SessionSettingsTests::ModifyFileSearchingPreference() const
{
    constexpr auto searchFiles = true;

    Settings::SessionSettings manager;
    manager.SearchFiles(searchFiles);

    QCOMPARE(manager.ShouldSearchFiles(), searchFiles);
}

void SessionSettingsTests::ModifyDirectorySearchingPreference() const
{
    constexpr auto searchDirectories = false;

    Settings::SessionSettings manager;
    manager.SearchDirectories(searchDirectories);

    QCOMPARE(manager.ShouldSearchDirectories(), searchDirectories);
}

void SessionSettingsTests::ModifyRegexSearchingPreference() const
{
    constexpr auto useRegex = true;

    Settings::SessionSettings manager;
    manager.UseRegexSearch(useRegex);

    QCOMPARE(manager.ShouldUseRegex(), useRegex);
}

void SessionSettingsTests::ModifyNumericPrefix() const
{
    constexpr auto numericPrefix = Constants::SizePrefix::Binary;

    Settings::SessionSettings manager;
    manager.SetActiveNumericPrefix(numericPrefix);

    QCOMPARE(manager.GetActiveNumericPrefix(), numericPrefix);
}

void SessionSettingsTests::ModifyVisualizationOptions() const
{
    using namespace Literals::Numeric::Binary;

    Settings::VisualizationOptions options;
    options.minimumFileSize = 1_MiB;
    options.onlyShowDirectories = true;

    Settings::SessionSettings manager;
    manager.SetVisualizationOptions(options);

    const auto& retrievedOptions = manager.GetVisualizationOptions();

    QCOMPARE(retrievedOptions.forceNewScan, options.forceNewScan);
    QCOMPARE(retrievedOptions.rootDirectory, options.rootDirectory);
    QCOMPARE(retrievedOptions.minimumFileSize, options.minimumFileSize);
    QCOMPARE(retrievedOptions.onlyShowDirectories, options.onlyShowDirectories);
}

REGISTER_TEST(SessionSettingsTests)
