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
    void VerifyFilesAreCreatedWhenMissing() const;

    /**
     * @brief SavingSettingsToDisk
     */
    void SavingSettingsToDisk() const;
};

#endif // PERSISTENTSETTINGSTESTS_H
