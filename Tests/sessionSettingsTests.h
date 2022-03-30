#ifndef SESSIONSETTINGSTESTS_H
#define SESSIONSETTINGSTESTS_H

#include <QtTest>

#include "Utilities/multiTestHarness.h"

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

    /**
     * @brief Verify that the setting governing whether files should be searched can be set and
     * retrieved correctly.
     */
    void ModifyFileSearchingPreference() const;

    /**
     * @brief Verify that the setting governing whether directories should be searched can be set
     * and retrieved correctly.
     */
    void ModifyDirectorySearchingPreference() const;

    /**
     * @brief Verify that the setting governing whether to use regex can be set and retrieved
     * successfully.
     */
    void ModifyRegexSearchingPreference() const;

    /**
     * @brief Verify that the active numeric prefix can be set and retrieved correctly.
     */
    void ModifyNumericPrefix() const;

    /**
     * @brief Verify visualization options can correctly set and retrieved.
     */
    void ModifyVisualizationOptions() const;
};

#endif // SESSIONSETTINGSTESTS_H
