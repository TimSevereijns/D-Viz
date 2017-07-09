#include "constants.h"
#include "Windows/mainWindow.h"

#include <QApplication>
#include <spdlog/spdlog.h>

template<typename NodeDataType>
class Tree;

struct VizFile;

int main(int argc, char* argv[])
{
#ifdef Q_OS_WIN
   spdlog::basic_logger_mt(Constants::Logging::LOG_NAME, ".\\log.txt");
#else
   spdlog::basic_logger_mt(Constants::Logging::LOG_NAME, "./log.txt");
#endif

   spdlog::get(Constants::Logging::LOG_NAME)->info("Starting D-Viz...");

   qRegisterMetaType<std::uintmax_t>("std::uintmax_t");
   qRegisterMetaType<std::shared_ptr<Tree<VizFile>>>("std::shared_ptr<Tree<VizFile>>");

   QApplication application{ argc, argv };

   Controller controller{ };
   MainWindow window{ controller, nullptr };
   controller.SetView(&window);
   window.show();

   const auto exitCode = application.exec();

   spdlog::get("D-Viz")->info("Exiting...");
   return exitCode;
}
