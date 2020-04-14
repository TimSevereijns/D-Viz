#include "controllerTests.h"

#include "Mocks/mockView.h"

#include <Monitor/fileMonitorBase.h>
#include <Visualizations/squarifiedTreemap.h>
#include <controller.h>

#include <filesystem>
#include <memory>

void ControllerTests::initTestCase()
{
}

void ControllerTests::cleanupTestCase()
{
}

void ControllerTests::init()
{
}

void ControllerTests::Foo() const
{
    ControllerParameters parameters;

    parameters.createView = [](Controller& controller) {
        return std::make_shared<MockView>(controller);
    };

    parameters.createModel = [](std::unique_ptr<FileMonitorBase> fileMonitor,
                                const std::filesystem::path& path) {
        return std::make_shared<SquarifiedTreeMap>(std::move(fileMonitor), path);
    };

    Controller controller{ parameters };
    controller.LaunchUI();
}

REGISTER_TEST(ControllerTests)
