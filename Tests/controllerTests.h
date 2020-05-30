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
    void SelectNode();

    /**
     * @brief Verifies that a selected node can be cleared.
     */
    void ClearSelectedNode();

    /**
     * @brief Verifies that files smaller than the minimum file size are not reported as
     * displayable.
     */
    void VerifyFilesOverLimitAreDisplayed() const;

    /**
     * @brief Verifies that files under the limit are not displayed.
     */
    void VerifyFilesUnderLimitAreNotDisplayed() const;

    /**
     * @brief Verifies that files are not displayed when only directories are allowed to be shown.
     */
    void VerifyFilesAreNotDisplayedWhenOnlyDirectoriesAllowed() const;

    /**
     * @brief Verifies that directories should not be shown when the size of that directory is too
     * small.
     */
    void VerifyDirectoriesUnderLimitAreNotShownWhenNotAllowed() const;

    /**
     * @brief Verifies that directories should not be shown when the size of that directory is
     * sufficiently large.
     */
    void VerifyDirectoriesOverLimitAreNotShownWhenNotAllowed() const;

    /**
     * @brief Verifies that searching the treemap returns the expected results and calls the
     * expected UI update functions.
     */
    void SearchTreemapWithoutPriorSelection() const;

    /**
     * @brief Verifies that searching the treemap returns the expected results and calls the
     * expected UI update functions when a prior highlight already exists.
     */
    void SearchTreemapWithPriorSelection() const;

    /**
     * @brief Verifies that ancestor nodes are correctly highlighted.
     */
    void HighlightAncestors() const;

    /**
     * @brief Verifies that a highlighted node is indeed reported as being highlighted.
     */
    void IsNodeHighlighted() const;

    /**
     * @brief Verifies that descendant nodes are correctly highlighted.
     */
    void HighlightDescendants() const;

    /**
     * @brief Verifies that a node can be selected via a picking ray.
     */
    void SelectNodeViaRay() const;

    /**
     * @brief Verifies that the default color of an unhighlighted/unselected node is what we expect.
     */
    void DetermineDefaultLeafNodeColor() const;

    /**
     * @brief Verifies that the default color of highlighted node is what we expect it to be.
     */
    void DetermineDefaultColorOfHighlightedNode() const;

    /**
     * @brief Verifies a non-default node color is properly tracked.
     */
    void DetermineCustomColorOfRegisteredNode() const;

  private:
    std::shared_ptr<TestViewFactory> m_viewFactory;
    std::shared_ptr<TestModelFactory> m_modelFactory;

    std::shared_ptr<MockView> m_view;
    std::shared_ptr<Controller> m_controller;
};

#endif // CONTROLLERTESTS_H
