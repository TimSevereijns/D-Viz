#ifndef PERSISTENTSETTINGSTESTS_H
#define PERSISTENTSETTINGSTESTS_H

#include <QtTest>

#include "multiTestHarness.h"

class PersistentSettingsTests : public QObject
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
     * @brief Verifies that the missing preference files are created if they don't already exist.
     */
    void VerifyFilesAreCreatedWhenAbsent() const;

    /**
     * @brief Verifies that settings can be correctly saved to disk and read back.
     */
    void SavingSettingsToDisk() const;

    /**
     * @brief Verifies that filesystem monitoring can be correctly toggled on and off.
     */
    void ToggleFileMonitoring() const;

    /**
     * @brief Verifies that the shadowing setting can be correctly toggled.
     */
    void ToggleShadowRendering() const;

    /**
     * @brief Verifies that the cascade split rendering setting can be correctly toggled.
     */
    void ToggleCascadeSplitRendering() const;

    /**
     * @brief Verifies that the origin rendering setting can be correctly toggled.
     */
    void ToggleOriginRendering() const;

    /**
     * @brief Verifies that the grid rendering setting can be correctly toggled.
     */
    void ToggleGridRendering() const;

    /**
     * @brief Verifies that the light marker rendering setting can be correctly toggled.
     */
    void ToggleLightMarkerRendering() const;

    /**
     * @brief Verifies that the frusta rendering setting can be correctly toggled.
     */
    void ToggleFrustaRendering() const;

    /**
     * @brief Verifies that the shadow map cascade counts can be correctly modified.
     */
    void ModifyShadowMapCascadeCount() const;

    /**
     * @brief Verifies that the shadow map cascade counts can be correctly modified, and that
     * the value remains within a certain range.
     */
    void ClampShadowMapCascadeCount() const;

    /**
     * @brief Verifies that the shadow map qualtiy can be correctly modified.
     */
    void ModifyShadowMapQuality() const;

    /**
     * @brief Verifies that the shadow map qualtiy can be correctly modified, and that
     * the value remains within a certain range.
     */
    void ClampShadowMapQuality() const;

    /**
     * @brief Verifies that the debugging menu can be correctly turned on and off.
     */
    void DebugMenuIsOffByDefault() const;

    /**
     * @brief Verifies that existing settings will be read from disk when the PersistentSettings
     * manager class is created.
     */
    void LoadSettingsFromDisk() const;
};

#endif // PERSISTENTSETTINGSTESTS_H
