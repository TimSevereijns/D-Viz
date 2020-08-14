#ifndef FILESTATUSCHANGE_HPP
#define FILESTATUSCHANGE_HPP

#include <chrono>
#include <filesystem>
#include <functional>

#include "constants.h"

#include <Tree/Tree.hpp>
#include <spdlog/spdlog.h>

struct VizBlock;

enum class FileEventType
{
    None,
    Created,
    Deleted,
    Touched,
    Renamed
};

struct FileEvent
{
    FileEvent() = default;

    FileEvent(std::filesystem::path path, FileEventType eventType)
        : path{ std::move(path) }, eventType{ eventType }
    {
        try {
            if (std::filesystem::is_regular_file(path)) {
                fileSize = std::filesystem::file_size(path);
            }
        } catch (const std::filesystem::filesystem_error& /*exception*/) {
            spdlog::get(Constants::Logging::FilesystemLog)
                ->error(fmt::format("Failed to obtain size of \"{}\"", path.string()));
        }
    }

    std::filesystem::path path;
    std::uint32_t eventId = 0u;
    std::uintmax_t fileSize = 0u;

    FileEventType eventType;
};

namespace std
{
    template <> struct less<std::filesystem::path>
    {
        /**
         * @returns True if the left-hand side argument is less than the right-hand side argument.
         */
        bool operator()(const std::filesystem::path& lhs, const std::filesystem::path& rhs) const
        {
            return lhs.native() < rhs.native();
        }
    };

    template <> struct hash<std::filesystem::path>
    {
        /**
         * @returns A hash based on the path of the changed file.
         */
        std::size_t operator()(const std::filesystem::path& path) const
        {
            return std::hash<std::filesystem::path::string_type>{}(path.native());
        }
    };
} // namespace std

#endif // FILESTATUSCHANGE_HPP
