#include "bootstrapper.hpp"
#include "controller.h"

#include <QApplication>
#include <spdlog/spdlog.h>

int main(int argc, char* argv[])
{
    Bootstrapper::RegisterMetaTypes();
    Bootstrapper::InitializeLogs();

    QApplication application{ argc, argv };

    Controller controller{};
    controller.LaunchUI();

    const auto exitCode = QApplication::exec();
    spdlog::get(Constants::Logging::DEFAULT_LOG)->info("Exiting...");
    return exitCode;
}
