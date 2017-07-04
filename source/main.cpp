#include "Windows/mainWindow.h"

#include "spdlog/spdlog.h"

#include <QApplication>

const auto& LOG_NAME{ "D-Viz" };

template<typename NodeDataType>
class Tree;

struct VizFile;

int main(int argc, char* argv[])
{
   spdlog::basic_logger_mt("D-Viz", ".\\log.txt");
   spdlog::get("D-Viz")->info("Starting D-Viz...");

   qRegisterMetaType<std::uintmax_t>("std::uintmax_t");
   qRegisterMetaType<std::shared_ptr<Tree<VizFile>>>("std::shared_ptr<Tree<VizFile>>");

   QApplication application{ argc, argv };

   Controller controller{ };
   MainWindow window{ controller, nullptr };
   controller.SetView(&window);
   window.show();

   const auto exitCode = application.exec();

   spdlog::get("D-Viz")->info("Quit cleanly!");
   return exitCode;
}
