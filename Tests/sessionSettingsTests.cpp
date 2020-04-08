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
    const auto desiredSpeed = 50;

    Settings::SessionSettings manager;
    manager.OnCameraSpeedChanged(desiredSpeed);

    QCOMPARE(manager.GetCameraSpeed(), desiredSpeed);
}

void SessionSettingsTests::ModifyMouseSensitivity() const
{
    const auto desiredSensitivity = 50;

    Settings::SessionSettings manager;
    manager.OnMouseSensitivityChanged(desiredSensitivity);

    QCOMPARE(manager.GetMouseSensitivity(), desiredSensitivity);
}

void SessionSettingsTests::ModifyLightAttenuationFactor() const
{
    const auto desiredFactor = 50;

    Settings::SessionSettings manager;
    manager.OnLightAttenuationChanged(desiredFactor);

    QCOMPARE(manager.GetLightAttenuationFactor(), desiredFactor);
}

void SessionSettingsTests::ModifyAmbientLightCoefficient() const
{
    const auto desiredCoefficient = 50;

    Settings::SessionSettings manager;
    manager.OnAmbientLightCoefficientChanged(desiredCoefficient);

    QCOMPARE(manager.GetAmbientLightCoefficient(), desiredCoefficient);
}

void SessionSettingsTests::ModifyPrimaryLightAttachmentToCamera() const
{
    const auto desiredAttachment = true;

    Settings::SessionSettings manager;
    manager.OnAttachLightToCameraStateChanged(desiredAttachment);

    QCOMPARE(manager.IsPrimaryLightAttachedToCamera(), desiredAttachment);
}

void SessionSettingsTests::ModifyFileSearchingPreference() const
{
    const auto searchFiles = true;

    Settings::SessionSettings manager;
    manager.OnShouldSearchFilesChanged(searchFiles);

    QCOMPARE(manager.ShouldSearchFiles(), searchFiles);
}

void SessionSettingsTests::ModifyDirectorySearchingPreference() const
{
    const auto searchDirectories = false;

    Settings::SessionSettings manager;
    manager.OnShouldSearchDirectoriesChanged(searchDirectories);

    QCOMPARE(manager.ShouldSearchDirectories(), searchDirectories);
}

void SessionSettingsTests::ModifyNumericPrefix() const
{
    const auto numericPrefix = Constants::FileSize::Prefix::BINARY;

    Settings::SessionSettings manager;
    manager.SetActiveNumericPrefix(Constants::FileSize::Prefix::BINARY);

    QCOMPARE(manager.GetActiveNumericPrefix(), numericPrefix);
}

void SessionSettingsTests::ModifyVisualizationParameters() const
{
    using namespace Literals::Numeric::Binary;

    Settings::VisualizationParameters parameters;
    parameters.minimumFileSize = 1_MiB;
    parameters.onlyShowDirectories = true;

    Settings::SessionSettings manager;
    manager.SetVisualizationParameters(parameters);

    const auto& retreivedManager = manager.GetVisualizationParameters();

    QCOMPARE(retreivedManager.forceNewScan, parameters.forceNewScan);
    QCOMPARE(retreivedManager.rootDirectory, parameters.rootDirectory);
    QCOMPARE(retreivedManager.minimumFileSize, parameters.minimumFileSize);
    QCOMPARE(retreivedManager.onlyShowDirectories, parameters.onlyShowDirectories);
}

void SessionSettingsTests::VerifyFilesOverLimitAreDisplayed() const
{
    using namespace Literals::Numeric::Binary;

    Settings::VisualizationParameters parameters;
    parameters.minimumFileSize = 1_KiB;
    parameters.onlyShowDirectories = false;

    VizBlock sample;
    sample.file.name = L"Foo";
    sample.file.extension = L".txt";
    sample.file.size = 16_KiB;
    sample.file.type = FileType::REGULAR;

    Settings::SessionSettings manager;
    manager.SetVisualizationParameters(parameters);
    const auto shouldDisplay = manager.IsBlockVisible(sample);

    QCOMPARE(shouldDisplay, true);
}

void SessionSettingsTests::VerifyFilesUnderLimitAreNotDisplayed() const
{
    using namespace Literals::Numeric::Binary;

    Settings::VisualizationParameters parameters;
    parameters.minimumFileSize = 32_KiB;
    parameters.onlyShowDirectories = false;

    VizBlock sample;
    sample.file.name = L"Foo";
    sample.file.extension = L".txt";
    sample.file.size = 16_KiB;
    sample.file.type = FileType::REGULAR;

    Settings::SessionSettings manager;
    manager.SetVisualizationParameters(parameters);
    const auto shouldDisplay = manager.IsBlockVisible(sample);

    QCOMPARE(shouldDisplay, false);
}

void SessionSettingsTests::VerifyFilesAreNotDisplayedWhenOnlyDirectoriesAllowed() const
{
    using namespace Literals::Numeric::Binary;

    Settings::VisualizationParameters parameters;
    parameters.minimumFileSize = 10_MiB;
    parameters.onlyShowDirectories = true;

    VizBlock sample;
    sample.file.name = L"Bar";
    sample.file.extension = L"";
    sample.file.size = 10_GiB;
    sample.file.type = FileType::REGULAR;

    Settings::SessionSettings manager;
    manager.SetVisualizationParameters(parameters);
    const auto shouldDisplay = manager.IsBlockVisible(sample);

    QCOMPARE(shouldDisplay, false);
}

void SessionSettingsTests::VerifyDirectoriesUnderLimitAreNotShownWhenNotAllowed() const
{
    using namespace Literals::Numeric::Binary;

    Settings::VisualizationParameters parameters;
    parameters.minimumFileSize = 1_MiB;
    parameters.onlyShowDirectories = true;

    VizBlock sample;
    sample.file.name = L"Bar";
    sample.file.extension = L"";
    sample.file.size = 10_MiB;
    sample.file.type = FileType::DIRECTORY;

    Settings::SessionSettings manager;
    manager.SetVisualizationParameters(parameters);
    const auto shouldDisplay = manager.IsBlockVisible(sample);

    QCOMPARE(shouldDisplay, true);
}

void SessionSettingsTests::VerifyDirectoriesOverLimitAreNotShownWhenNotAllowed() const
{
    using namespace Literals::Numeric::Binary;

    Settings::VisualizationParameters parameters;
    parameters.minimumFileSize = 10_MiB;
    parameters.onlyShowDirectories = true;

    VizBlock sample;
    sample.file.name = L"Bar";
    sample.file.extension = L"";
    sample.file.size = 1_MiB;
    sample.file.type = FileType::DIRECTORY;

    Settings::SessionSettings manager;
    manager.SetVisualizationParameters(parameters);
    const auto shouldDisplay = manager.IsBlockVisible(sample);

    QCOMPARE(shouldDisplay, false);
}

REGISTER_TEST(SessionSettingsTests)
