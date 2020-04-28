#include "Monitor/fileMonitorBase.h"
#include "Visualizations/squarifiedTreemap.h"
#include "bootstrapper.hpp"
#include "controller.h"

#include <filesystem>
#include <memory>
#include <string>

#include <QApplication>
#include <spdlog/spdlog.h>

int main(int argc, char* argv[])
{
    Bootstrapper::RegisterMetaTypes();
    Bootstrapper::InitializeLogs();

    QApplication application{ argc, argv };

#if defined(Q_OS_WIN)
    application.setWindowIcon(QIcon{ "Icons/Windows/D-Viz.ico" });
#elif defined(Q_OS_LINUX)
    application.setWindowIcon(QIcon{ "Icons/Linux/32x32/D-Viz.png" });
#endif

    auto viewFactory = ViewFactory{};
    auto modelFactory = ModelFactory{};

    Controller controller{ viewFactory, modelFactory };
    controller.LaunchUI();

    const auto exitCode = QApplication::exec();
    spdlog::get(Constants::Logging::DefaultLog)->info("Exiting...");
    return exitCode;
}
