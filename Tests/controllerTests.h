#ifndef CONTROLLERTESTS_H
#define CONTROLLERTESTS_H

#include <QObject>

#include "multiTestHarness.h"

class ControllerTests : public QObject
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
     * @brief Foo
     */
    void Foo() const;
};

#endif // CONTROLLERTESTS_H
