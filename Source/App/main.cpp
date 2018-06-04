#include "constants.h"

#include "controller.h"
#include "Utilities/ignoreUnused.hpp"
#include "Visualizations/windowsFileMonitor.h"

#include <QApplication>
#include <spdlog/spdlog.h>

#undef RAPIDJSON_HAS_STDSTRING
#define RAPIDJSON_HAS_STDSTRING 1

template<typename NodeDataType>
class Tree;

struct VizBlock;

namespace
{
   /**
    * @brief Returns a wide string if on Windows, and returns a narrow string on Unix.
   */
   auto ToFilenameString(const std::experimental::filesystem::path& path)
   {
      if constexpr (std::is_same_v<spdlog::filename_t, std::wstring>)
      {
         return path.wstring();
      }

      if constexpr (std::is_same_v<spdlog::filename_t, std::string>)
      {
         return path.string();
      }
   }

   /**
    * @brief Performs all the steps necessary to initialize and start the log.
    */
   void InitializeLog()
   {
      const auto defaultLogPath = std::experimental::filesystem::current_path().append("log.txt");

      const auto& defaultLog = spdlog::basic_logger_mt(
         Constants::Logging::DEFAULT_LOG,
         ToFilenameString(defaultLogPath));

      const auto fileLogPath = std::experimental::filesystem::current_path().append("fileSytem.txt");

      const auto& filesystemLog = spdlog::basic_logger_mt(
         Constants::Logging::FILESYSTEM_LOG,
         ToFilenameString(fileLogPath));

      defaultLog->info("--------------------------------");
      defaultLog->info("Starting D-Viz...");

      filesystemLog->info("--------------------------------");
      filesystemLog->info("Starting D-Viz...");
   }

   /**
    * @brief Registers the types that we'd like pass through the Qt signaling framework.
    */
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
