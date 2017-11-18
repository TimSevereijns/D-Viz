#ifndef SETTINGS_H
#define SETTINGS_H

#include <experimental/filesystem>
#include <memory>

#include <rapidjson/document.h>
#include <rapidjson/encodings.h>

namespace Settings
{
   using JsonDocument = rapidjson::GenericDocument<rapidjson::UTF16<wchar_t>>;

   JsonDocument ParseJsonDocument(const std::experimental::filesystem::path& path);

   /**
    * @brief The VisualizationParameters struct represents the gamut of visualization parameters that
    * can be set to control when visualization updates occur, as well as what nodes get included.
    */
   struct VisualizationParameters
   {
      std::wstring rootDirectory{ L"" };  ///< The path to the root directory

      std::uint64_t minimumFileSize{ 0 }; ///< The minimum size a file should be before it shows up.

      bool forceNewScan{ true };          ///< Whether a new scan should take place.
      bool onlyShowDirectories{ false };  ///< Whether only directories should be shown.
      bool useDirectoryGradient{ false }; ///< Whether to use gradient coloring for the directories.
   };
}

#endif // SETTINGS_H
