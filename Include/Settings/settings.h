#ifndef SETTINGS_H
#define SETTINGS_H

#include <filesystem>
#include <string>

#include <rapidjson/document.h>
#include <rapidjson/encodings.h>

class VizBlock;

namespace Settings
{
    using JsonDocument = rapidjson::Document;

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
} // namespace Settings

#endif // SETTINGS_H
