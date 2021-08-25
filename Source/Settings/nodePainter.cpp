#include "Settings/nodePainter.h"

#include <Model/vizBlock.h>
#include <constants.h>

#include <spdlog/spdlog.h>

#ifdef Q_OS_WIN
// Conflicts with Rapid JSON API.
#undef GetObject
#endif // Q_OS_WIN

namespace
{
    /**
     * @brief Populates the passed in map with the flattened content of the JSON document.
     *
     * @param[in] json               The JSON document containing the file color information.
     * @param[out] map               The map that is to contain the flattened JSON data.
     */
    void
    PopulateColorMapFromJsonDocument(const Settings::JsonDocument& json, Settings::ColorMap& map)
    {
        if (!json.IsObject()) {
            return;
        }

        auto encounteredError = false;

        for (const auto& category : json.GetObject()) {
            if (!category.value.IsObject()) {
                encounteredError = true;
                continue;
            }

            std::unordered_map<std::string, QVector3D> extensionMap;

            for (const auto& extension : category.value.GetObject()) {
                if (!extension.value.IsArray()) {
                    encounteredError = true;
                    continue;
                }

                const auto colorArray = extension.value.GetArray();
                QVector3D colorVector{ colorArray[0].GetInt() / 255.0f,
                                       colorArray[1].GetInt() / 255.0f,
                                       colorArray[2].GetInt() / 255.0f };

                extensionMap.emplace(extension.name.GetString(), colorVector);
            }

            map.emplace(category.name.GetString(), std::move(extensionMap));
        }

        if (encounteredError) {
            const auto& log = spdlog::get(Constants::Logging::DefaultLog);
            log->error("Encountered an error converting JSON document to file color map.");
        }
    }

    template <typename Container>
    rapidjson::Value
    ConstructColorMap(const Container& fileTypes, Settings::JsonDocument::AllocatorType& allocator)
    {
        using JsonValue = rapidjson::Value;
        constexpr auto color = Constants::Colors::SchemeHighlight;

        JsonValue array{ rapidjson::kArrayType };
        array.PushBack(static_cast<int>(color[0] * 255), allocator);
        array.PushBack(static_cast<int>(color[1] * 255), allocator);
        array.PushBack(static_cast<int>(color[2] * 255), allocator);

        JsonValue object{ rapidjson::kObjectType };
        for (const auto& type : fileTypes) {
            object.AddMember(
                JsonValue{ type.data(), allocator }, JsonValue{}.CopyFrom(array, allocator),
                allocator);
        }

        return object;
    }

    void AddVideoColors(
        Settings::JsonDocument& document, Settings::JsonDocument::AllocatorType& allocator)
    {
        constexpr std::array<std::string_view, 18> fileTypes = {
            ".mp2",  ".mp4", ".avi", ".mkv", ".mov", ".vob", ".gifv", ".wmv",  ".mpg",
            ".mpeg", ".m4v", ".flv", ".mpv", ".ogv", ".ogg", ".mts",  ".m2ts", ".ts"
        };

        auto colorMap = ConstructColorMap(fileTypes, allocator);
        document.AddMember("Videos", colorMap.Move(), allocator);
    }

    void AddImageColors(
        Settings::JsonDocument& document, Settings::JsonDocument::AllocatorType& allocator)
    {
        constexpr std::array<std::string_view, 28> fileTypes = {
            ".jpg", ".jpeg", ".tiff", ".png", ".psd", ".gif", ".bmp", ".svg", ".dng", ".RAF",
            ".crw", ".cr2",  ".cr3",  ".cap", ".erf", ".fff", ".gpr", ".mef", ".mdc", ".mrw",
            ".nef", ".nrw",  ".orf",  ".orf", ".pef", ".ptx", ".x3f", ".srw"
        };

        auto colorMap = ConstructColorMap(fileTypes, allocator);
        document.AddMember("Images", colorMap.Move(), allocator);
    }
} // namespace

namespace Settings
{
    NodePainter::NodePainter(const std::filesystem::path& colorFile)
        : m_fileColorMapPath{ colorFile }
    {
        if (!std::filesystem::exists(m_fileColorMapPath)) {
            m_fileColorMapDocument = CreateColorsDocument();
        } else {
            m_fileColorMapDocument = LoadFromDisk(m_fileColorMapPath);
        }

        PopulateColorMapFromJsonDocument(m_fileColorMapDocument, m_colorMap);
    }

    const ColorMap& NodePainter::GetFileColorMap() const
    {
        return m_colorMap;
    }

    const std::string& NodePainter::GetActiveColorScheme() const
    {
        return m_colorScheme;
    }

    void NodePainter::SetActiveColorScheme(std::string_view scheme)
    {
        m_colorScheme = scheme;
    }

    void NodePainter::RegisterColorScheme(
        const std::string& name, const std::unordered_map<std::string, QVector3D>& map)
    {
        m_colorMap.insert_or_assign(name, map);
    }

    std::optional<QVector3D>
    NodePainter::DetermineColorFromExtension(std::string_view extension) const
    {
        const auto categoryItr = m_colorMap.find(m_colorScheme);
        if (categoryItr == std::end(m_colorMap)) {
            return std::nullopt;
        }

        const auto extensionItr = categoryItr->second.find(extension.data());
        if (extensionItr == std::end(categoryItr->second)) {
            return std::nullopt;
        }

        return extensionItr->second;
    }

    JsonDocument NodePainter::CreateColorsDocument()
    {
        Settings::JsonDocument document;
        document.SetObject();

        auto& allocator = document.GetAllocator();

        AddVideoColors(document, allocator);
        AddImageColors(document, allocator);

        Settings::SaveToDisk(document, std::filesystem::current_path() / L"colors.json");

        return document;
    }
} // namespace Settings
