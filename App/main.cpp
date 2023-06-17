#include "Factories/modelFactory.h"
#include "Factories/viewFactory.h"
#include "bootstrapper.h"
#include "constants.h"
#include "controller.h"

#include <QApplication>
#include <spdlog/spdlog.h>

int main(int argc, char* argv[])
{
    std::ignore = std::locale::global(std::locale{ "en_US.UTF-8" });

    Bootstrapper::RegisterMetaTypes();
    Bootstrapper::InitializeLogs();

    QSurfaceFormat format;
    format.setDepthBufferSize(32);
    format.setSwapInterval(0); //< Disable v-sync
    format.setSamples(8);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    format.setMajorVersion(4);
    format.setMinorVersion(1);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setOption(QSurfaceFormat::DebugContext);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
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
