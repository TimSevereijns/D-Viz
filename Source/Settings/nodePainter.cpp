#include "Settings/nodePainter.h"

#include <Model/vizBlock.h>
#include <constants.h>

#include <spdlog/spdlog.h>

#ifdef Q_OS_WIN
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

            std::unordered_map<std::wstring, QVector3D> extensionMap;

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

    void AddVideoColors(
        Settings::JsonDocument& document, Settings::JsonDocument::AllocatorType& allocator)
    {
        using JsonValue = rapidjson::GenericValue<rapidjson::UTF16<>>;
        constexpr std::array<std::wstring_view, 10> fileTypes = { L".mp4", L".avi", L".mkv",
                                                                  L".mov", L".vob", L".gifv",
                                                                  L".wmv", L".mpg", L".mpeg",
                                                                  L".m4v" };

        JsonValue array{ rapidjson::kArrayType };
        array.PushBack(static_cast<int>(Constants::Colors::GrayBlue[0] * 255), allocator);
        array.PushBack(static_cast<int>(Constants::Colors::GrayBlue[1] * 255), allocator);
        array.PushBack(static_cast<int>(Constants::Colors::GrayBlue[2] * 255), allocator);

        JsonValue object{ rapidjson::kObjectType };
        for (const auto& type : fileTypes) {
            object.AddMember(
                JsonValue{ type.data(), allocator }, JsonValue{}.CopyFrom(array, allocator),
                allocator);
        }

        document.AddMember(L"Videos", object.Move(), allocator);
    }

    void AddImageColors(
        Settings::JsonDocument& document, Settings::JsonDocument::AllocatorType& allocator)
    {
        using JsonValue = rapidjson::GenericValue<rapidjson::UTF16<>>;
        constexpr std::array<std::wstring_view, 8> fileTypes = { L".jpg", L".jpeg", L".tiff",
                                                                 L".png", L".psd",  L".gif",
                                                                 L".bmp", L".svg" };

        JsonValue array{ rapidjson::kArrayType };
        array.PushBack(static_cast<int>(Constants::Colors::GrayBlue[0] * 255), allocator);
        array.PushBack(static_cast<int>(Constants::Colors::GrayBlue[1] * 255), allocator);
        array.PushBack(static_cast<int>(Constants::Colors::GrayBlue[2] * 255), allocator);

        JsonValue object{ rapidjson::kObjectType };
        for (const auto& type : fileTypes) {
            object.AddMember(
                JsonValue{ type.data(), allocator }, JsonValue{}.CopyFrom(array, allocator),
                allocator);
        }

        document.AddMember(L"Images", object.Move(), allocator);
    }
} // namespace

namespace Settings
{
    NodePainter::NodePainter(const std::filesystem::path& colorFile)
        : m_fileColorMapPath{ colorFile }
    {
        if (!std::filesystem::exists(m_fileColorMapPath)) {
            m_fileColorMapDocument = CreatePreferencesDocument();
        } else {
            m_fileColorMapDocument = LoadFromDisk(m_fileColorMapPath);
        }

        PopulateColorMapFromJsonDocument(m_fileColorMapDocument, m_colorMap);
    }

    const ColorMap& NodePainter::GetFileColorMap() const
    {
        return m_colorMap;
    }

    const std::wstring& NodePainter::GetActiveColorScheme() const
    {
        return m_colorScheme;
    }

    void NodePainter::SetColorScheme(std::wstring_view scheme)
    {
        m_colorScheme = scheme;
    }

    std::optional<QVector3D>
    NodePainter::DetermineColorFromExtension(std::wstring_view extension) const
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

    JsonDocument NodePainter::CreatePreferencesDocument()
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
