#include "Factories/modelFactory.h"
#include "Factories/viewFactory.h"
#include "bootstrapper.hpp"
#include "constants.h"
#include "controller.h"

#include <QApplication>
#include <spdlog/spdlog.h>

int main(int argc, char* argv[])
{
    Bootstrapper::RegisterMetaTypes();
    Bootstrapper::InitializeLogs();

    QApplication application{ argc, argv };

#if defined(Q_OS_WIN)
    QApplication::setWindowIcon(QIcon{ "Icons/Windows/D-Viz.ico" });
#elif defined(Q_OS_LINUX)
    QApplication::setWindowIcon(QIcon{ "Icons/Linux/32x32/D-Viz.png" });
#endif

    ViewFactory viewFactory;
    ModelFactory modelFactory;

    Controller controller{ viewFactory, modelFactory };
    controller.LaunchUI();

    const auto exitCode = QApplication::exec();
    spdlog::get(Constants::Logging::DefaultLog)->info("Exiting...");
    return exitCode;
}
