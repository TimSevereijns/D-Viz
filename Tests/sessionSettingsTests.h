#ifndef SESSIONSETTINGSTESTS_H
#define SESSIONSETTINGSTESTS_H

#include <QtTest>

#include "multiTestHarness.h"

class SessionSettingsTests : public QObject
{
    Q_OBJECT

  private slots:

    /**
     * @brief This preamble is run only once for the entire class. All setup work should be done
     * here.
     */
    void initTestCase();

    /**
     * @brief Clean up code for the entire class; called once.
     */
    void cleanupTestCase();

    /**
     * @brief This preamble is run before each test.
     */
    void init();

    /**
     * @brief Verify that camera speed can be set and retrieved correctly.
     */
    void ModifyCameraSpeed() const;

    /**
     * @brief Verify that mouse sensitivity can be set and retrieved correctly.
     */
    void ModifyMouseSensitivity() const;

    /**
     * @brief Verify that the light attenuation factor can be set and retrieved correctly.
     */
    void ModifyLightAttenuationFactor() const;

    /**
     * @brief Verify that the ambient light coefficient can be set and retrieved correctly.
     */
    void ModifyAmbientLightCoefficient() const;

    /**
     * @brief Verify that the attachment state of the primary light to the camera can be set and
     * retrieved correctly.
     */
    void ModifyPrimaryLightAttachmentToCamera() const;
};

#endif // SESSIONSETTINGSTESTS_H
