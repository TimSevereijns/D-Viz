#include "constants.h"

#include "controller.h"
#include "Utilities/ignoreUnused.hpp"

#include <QApplication>
#include <spdlog/spdlog.h>

#undef RAPIDJSON_HAS_STDSTRING
#define RAPIDJSON_HAS_STDSTRING 1

template<typename NodeDataType>
class Tree;

struct VizBlock;

namespace
{
   void InitializeLog()
   {
#ifdef Q_OS_WIN
      spdlog::basic_logger_mt(Constants::Logging::DEFAULT_LOG, ".\\log.txt");
#else
      spdlog::basic_logger_mt(Constants::Logging::DEFAULT_LOG, "./log.txt");
#endif

      const auto& log = spdlog::get(Constants::Logging::DEFAULT_LOG);
      log->info("--------------------------------");
      log->info("Starting D-Viz...");
   }

   void RegisterMetaTypes()
   {
      qRegisterMetaType<std::uintmax_t>("std::uintmax_t");
      qRegisterMetaType<std::shared_ptr<Tree<VizBlock>>>("std::shared_ptr<Tree<VizBlock>>");
   }
}

int main(int argc, char* argv[])
{
   RegisterMetaTypes();

   InitializeLog();

   QApplication application{ argc, argv };

   Controller controller{ };
   controller.LaunchUI();

   const auto exitCode = application.exec();

   spdlog::get(Constants::Logging::DEFAULT_LOG)->info("Exiting...");
   return exitCode;
}
