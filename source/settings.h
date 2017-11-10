#ifndef SETTINGS_H
#define SETTINGS_H

#include <experimental/filesystem>
#include <memory>

#include <rapidjson/document.h>
#include <rapidjson/encodings.h>

namespace Settings
{
   using JsonDocument = rapidjson::GenericDocument<rapidjson::UTF16<wchar_t>>;

   JsonDocument LoadColorSettingsFromDisk(const std::experimental::filesystem::path& path);
}

#endif // SETTINGS_H
