#ifndef NODEPAINTERTESTS_H
#define NODEPAINTERTESTS_H

#include <QObject>

#include "multiTestHarness.h"

class NodePainterTests : public QObject
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
     * @brief Verify that the serialized colors can be read from disk correctly.
     */
    void DetermineColorsFromSettingsOnDisk() const;

    /**
     * @brief Verify that retrieving a non-existant mapping returns an empty optional.
     */
    void GetBackEmptyOptionalOnEmptyMapping() const;

    /**
     * @brief Verify that color scheme names can be set the retrieved.
     */
    void ModifyActiveColorScheme() const;
};

#endif // NODEPAINTERTESTS_H
