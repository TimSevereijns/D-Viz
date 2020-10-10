#ifndef SETTINGS_H
#define SETTINGS_H

#include <filesystem>
#include <string>

#include <rapidjson/document.h>
#include <rapidjson/encodings.h>

class VizBlock;

namespace Settings
{
    using JsonDocument = rapidjson::GenericDocument<rapidjson::UTF16<wchar_t>>;

    /**
     * @brief Helper function to parse a JSON file on disk.
     *
     * @param[in] path               Path to the JSON file.
     *
     * @returns A parsed JSON document.
     */
    JsonDocument LoadFromDisk(const std::filesystem::path& path);

    /**
     * @brief Helper function to save a JSON file to disk.
     *
     * @param[in] document           The JSON document to save.
     * @param[in] path               The path on disk to which to save the document.
     *
     * @returns True if the save succeeded.
     */
    bool SaveToDisk(const JsonDocument& document, const std::filesystem::path& path);

    /**
     * @brief Visualization parameters that can be set to control when visualization updates occur,
     * as well as what nodes get included.
     */
    class VisualizationParameters
    {
      public:
        /**
         * @brief Determines whether a node is visible in the visualization.
         *
         * @param[in] block             The node whose visibility is to be checked.
         *
         * @returns True if the node in question is visible to the end-user.
         */
        bool IsNodeVisible(const VizBlock& block) const noexcept;

        std::wstring rootDirectory{ L"" }; ///< The path to the root directory

        // The minimum size a file should be before it shows up.
        std::uint64_t minimumFileSize{ 0 };

        bool forceNewScan{ true };         ///< Whether a new scan should take place.
        bool onlyShowDirectories{ false }; ///< Whether only directories should be shown.
    };
} // namespace Settings

#endif // SETTINGS_H
