#ifndef FILESTATUSCHANGE_HPP
#define FILESTATUSCHANGE_HPP

#include <chrono>
#include <filesystem>
#include <functional>

#include "constants.h"

#include <Tree/Tree.hpp>
#include <spdlog/spdlog.h>

enum class FileModification
{
    NONE,
    CREATED,
    DELETED,
    TOUCHED,
    RENAMED
};

struct VizBlock;

/**
 * @brief The FileEvent struct
 *
 * @todo File size should probably be added to the FileEvent struct. This will make
 * it far easier to unit test, since I can then more easily fake notifications.
 */
// struct FileEvent
//{
//    FileEvent() = default;

//    FileEvent(std::experimental::filesystem::path path, FileModification status)
//        : path{ std::move(path) }, status{ status }
//    {
//    }

//    // The relative path from the root of the visualization to the node that changed.
//    std::experimental::filesystem::path path;

//    // The type of change that occurred.
//    FileModification status{ FileModification::NONE };

//    // A pointer to the corresponding node in the tree, should it exist.
//    typename Tree<VizBlock>::Node* node{ nullptr }; //< @todo Should this be const?

//    friend bool operator==(const FileEvent& lhs, const FileEvent& rhs)
//    {
//        return lhs.node == rhs.node && lhs.path == rhs.path && lhs.status == rhs.status;
//    }
//};

enum class FileEventType
{
    NONE,
    CREATED,
    DELETED,
    TOUCHED,
    RENAMED
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