#include "Settings/settings.h"
#include "constants.h"

#include <fstream>
#include <ostream>

#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>

#include <spdlog/spdlog.h>

#include <Model/vizBlock.h>

namespace Settings
{
    JsonDocument LoadFromDisk(const std::filesystem::path& path)
    {
        std::ifstream fileStream{ path.string() };
        rapidjson::IStreamWrapper streamWrapper{ fileStream };

        JsonDocument document;
        document.ParseStream<rapidjson::kParseDefaultFlags, rapidjson::UTF8<char>>(streamWrapper);

        if (document.HasParseError()) {
            const auto* message = rapidjson::GetParseError_En(document.GetParseError());
            const auto& log = spdlog::get(Constants::Logging::DefaultLog);
            log->error(message);
        }

        return document;
    }

    bool SaveToDisk(const JsonDocument& document, const std::filesystem::path& path)
    {
        std::ofstream fileStream{ path.string() };
        rapidjson::OStreamWrapper streamWrapper{ fileStream };

        rapidjson::PrettyWriter<decltype(streamWrapper), rapidjson::UTF16<wchar_t>> writer{
            streamWrapper
        };

        const auto success = document.Accept(writer);

        if (!success) {
            const auto& log = spdlog::get(Constants::Logging::DefaultLog);
            log->error("Encountered error writing JSON document to \"{}\".", path.string());
        }

        return success;
    }

    bool VisualizationParameters::IsNodeVisible(const VizBlock& block) const
    {
        if (block.file.size < minimumFileSize) {
            return false;
        }

        if (block.file.type != FileType::Directory && onlyShowDirectories) {
            return false;
        }

        return true;
    }
} // namespace Settings
