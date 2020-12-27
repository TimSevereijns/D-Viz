#ifndef UITESTS_H
#define UITESTS_H

#include <QObject>

#include <Factories/modelFactory.h>
#include <Factories/viewFactory.h>

#include "Utilities/multiTestHarness.h"

class UiTests : public QObject
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
     * @brief Verify that the appropriate functions are called to launch the view.
     */
    void LaunchMainWindow() const;

  private:
    std::shared_ptr<ViewFactory> m_viewFactory;
    std::shared_ptr<ModelFactory> m_modelFactory;

    std::shared_ptr<Controller> m_controller;
};

#endif // UITESTS_H
