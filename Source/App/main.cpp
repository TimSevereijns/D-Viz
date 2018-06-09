#include "bootstrapper.hpp"
#include "constants.h"
#include "controller.h"

#include <QApplication>
#include <spdlog/spdlog.h>

int main(int argc, char* argv[])
{
   Bootstrapper::RegisterMetaTypes();
   Bootstrapper::InitializeLog();

   QApplication application{ argc, argv };

   Controller controller{ };
   controller.LaunchUI();

   const auto exitCode = application.exec();

   spdlog::get(Constants::Logging::DEFAULT_LOG)->info("Exiting...");
   return exitCode;
}
