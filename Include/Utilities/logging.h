#ifndef LOGGING_H
#define LOGGING_H

#include <filesystem>
#include <string>

namespace Logging
{
    inline std::filesystem::path GetDefaultLogPath(const std::string& suffix = std::string{})
    {
        const auto defaultLogName = "log" + suffix + ".txt";
        return std::filesystem::current_path() / defaultLogName;
    }

    inline std::filesystem::path GetFilesystemLogPath(const std::string& suffix = std::string{})
    {
        const auto filesystemLogName = "filesystem" + suffix + ".txt";
        return std::filesystem::current_path() / filesystemLogName;
    }
} // namespace Logging

#endif // LOGGING_H
