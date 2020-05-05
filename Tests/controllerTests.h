#ifndef CONTROLLERTESTS_H
#define CONTROLLERTESTS_H

#include <QObject>

#include <Factories/modelFactoryInterface.h>
#include <Factories/viewFactoryInterface.h>
#include <Monitor/fileMonitorBase.h>
#include <Visualizations/squarifiedTreemap.h>
#include <Windows/baseView.h>

#include "Mocks/mockView.h"
#include "Utilities/multiTestHarness.h"

class Controller;
class MockView;

class TestViewFactory final : public ViewFactoryInterface
{
  public:
    TestViewFactory(std::shared_ptr<MockView>& view) : m_view{ view }
    {
    }

    auto CreateView(Controller& controller) const -> std::shared_ptr<BaseView> override
    {
        m_view = std::make_shared<MockView>(controller);
        return m_view;
    }

  private:
    std::shared_ptr<MockView>& m_view;
};

class TestModelFactory final : public ModelFactoryInterface
{
  public:
    auto CreateModel(
        std::unique_ptr<FileMonitorBase> fileMonitor, const std::filesystem::path& path) const
        -> std::shared_ptr<BaseModel> override
    {
        return std::make_shared<SquarifiedTreeMap>(std::move(fileMonitor), path);
    }
};

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
     * @brief Verify that the appropriate functions are called to launch the view.
     */
    void LaunchMainWindow() const;

    /**
     * @brief Verify that the appropriate view and model functions are called when a scan is
     * performed.
     */
    void ScanDrive() const;

    /**
     * @brief Verify that the model is correctly reported as having been loaded.
     */
    void HasModelBeenLoaded() const;

    /**
     * @brief Verify that single node can be selected and retrieved.
     */
    void SelectingANode();

  private:
    std::shared_ptr<TestViewFactory> m_viewFactory;
    std::shared_ptr<TestModelFactory> m_modelFactory;

    std::shared_ptr<MockView> m_view;
    std::shared_ptr<Controller> m_controller;
};

#endif // CONTROLLERTESTS_H
