#ifndef BOOTSTRAPPER_HPP
#define BOOTSTRAPPER_HPP

#include "Utilities/logging.h"
#include "constants.h"

#include <filesystem>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#undef RAPIDJSON_HAS_STDSTRING
#define RAPIDJSON_HAS_STDSTRING 1

template <typename NodeDataType> class Tree;
class VizBlock;

namespace Bootstrapper
{
    /**
     * @brief Performs all the steps necessary to initialize and start the log.
     */
    inline void InitializeLogs(const std::string& suffix = std::string{})
    {
        const auto& defaultLog = spdlog::basic_logger_mt(
            Constants::Logging::DefaultLog, Logging::GetDefaultLogPath(suffix).string());

        const auto& filesystemLog = spdlog::basic_logger_mt(
            Constants::Logging::FilesystemLog, Logging::GetFilesystemLogPath(suffix).string());

        defaultLog->info("--------------------------------");
        defaultLog->info("Starting D-Viz...");

        filesystemLog->info("--------------------------------");
        filesystemLog->info("Starting D-Viz...");
    }

    /**
     * @brief Registers the types that we'd like pass through the Qt signaling framework.
     */
    inline void RegisterMetaTypes()
    {
        qRegisterMetaType<std::uintmax_t>("std::uintmax_t");
        qRegisterMetaType<std::shared_ptr<Tree<VizBlock>>>("std::shared_ptr<Tree<VizBlock>>");
    }
} // namespace Bootstrapper

#endif // BOOTSTRAPPER_HPP
