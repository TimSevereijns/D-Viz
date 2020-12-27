#include "uiTests.h"

#include "Utilities/testUtilities.h"

#include <controller.h>

#include <filesystem>
#include <memory>

void UiTests::initTestCase()
{
}

void UiTests::cleanupTestCase()
{
}

void UiTests::init()
{
    m_viewFactory = std::make_shared<ViewFactory>();
    m_modelFactory = std::make_shared<ModelFactory>();

    m_controller = std::make_shared<Controller>(*m_viewFactory, *m_modelFactory);
    m_controller->GetPersistentSettings().MonitorFileSystem(false);
}

void UiTests::LaunchMainWindow() const
{
    m_controller->LaunchUI();
}

REGISTER_TEST(UiTests)
