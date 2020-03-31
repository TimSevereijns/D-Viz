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

    QVERIFY(manager.GetCameraSpeed() == desiredSpeed);
}

void SessionSettingsTests::ModifyMouseSensitivity() const
{
    const auto desiredSensitivity = 50;

    Settings::SessionSettings manager;
    manager.OnMouseSensitivityChanged(desiredSensitivity);

    QVERIFY(manager.GetMouseSensitivity() == desiredSensitivity);
}

void SessionSettingsTests::ModifyLightAttenuationFactor() const
{
    const auto desiredFactor = 50;

    Settings::SessionSettings manager;
    manager.OnLightAttenuationChanged(desiredFactor);

    QVERIFY(manager.GetLightAttenuationFactor() == desiredFactor);
}

void SessionSettingsTests::ModifyAmbientLightCoefficient() const
{
    const auto desiredCoefficient = 50;

    Settings::SessionSettings manager;
    manager.OnAmbientLightCoefficientChanged(desiredCoefficient);

    QVERIFY(manager.GetAmbientLightCoefficient() == desiredCoefficient);
}

void SessionSettingsTests::ModifyPrimaryLightAttachmentToCamera() const
{
    const auto desiredAttachment = true;

    Settings::SessionSettings manager;
    manager.OnAttachLightToCameraStateChanged(desiredAttachment);

    QVERIFY(manager.IsPrimaryLightAttachedToCamera() == desiredAttachment);
}

void SessionSettingsTests::ModifyFileSearchingPreference() const
{
    const auto searchFiles = true;

    Settings::SessionSettings manager;
    manager.OnShouldSearchFilesChanged(searchFiles);

    QVERIFY(manager.ShouldSearchFiles() == searchFiles);
}

void SessionSettingsTests::ModifyDirectorySearchingPreference() const
{
    const auto searchDirectories = false;

    Settings::SessionSettings manager;
    manager.OnShouldSearchDirectoriesChanged(searchDirectories);

    QVERIFY(manager.ShouldSearchDirectories() == searchDirectories);
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

    QVERIFY(retreivedManager.forceNewScan == parameters.forceNewScan);
    QVERIFY(retreivedManager.rootDirectory == parameters.rootDirectory);
    QVERIFY(retreivedManager.minimumFileSize == parameters.minimumFileSize);
    QVERIFY(retreivedManager.onlyShowDirectories == parameters.onlyShowDirectories);
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

    QVERIFY(shouldDisplay == true);
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

    QVERIFY(shouldDisplay == false);
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

    QVERIFY(shouldDisplay == true);
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

    QVERIFY(shouldDisplay == false);
}

REGISTER_TEST(SessionSettingsTests)
