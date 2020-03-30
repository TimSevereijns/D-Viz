#include "sessionSettingsTests.h"

#include <Settings/sessionSettings.h>

#include "multiTestHarness.h"
#include "testUtilities.hpp"

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

REGISTER_TEST(SessionSettingsTests)
