#include "Settings/nodePainter.h"

#include <Visualizations/vizBlock.h>
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
                QVector3D colorVector{ colorArray[0].GetFloat() / 255.0f,
                                       colorArray[1].GetFloat() / 255.0f,
                                       colorArray[2].GetFloat() / 255.0f };

                extensionMap.emplace(extension.name.GetString(), colorVector);
            }

            map.emplace(category.name.GetString(), std::move(extensionMap));
        }

        if (encounteredError) {
            const auto& log = spdlog::get(Constants::Logging::DefaultLog);
            log->error("Encountered an error converting JSON document to file color map.");
        }
    }
} // namespace

namespace Settings
{
    NodePainter::NodePainter(const std::filesystem::path& colorFile)
        : m_fileColorMapPath{ colorFile }
    {
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

    void NodePainter::SetColorScheme(const std::wstring& scheme)
    {
        m_colorScheme = scheme;
    }

    std::optional<QVector3D>
    NodePainter::DetermineColorFromExtension(const Tree<VizBlock>::Node& node) const
    {
        const auto categoryItr = m_colorMap.find(m_colorScheme);
        if (categoryItr == std::end(m_colorMap)) {
            return std::nullopt;
        }

        const auto extensionItr = categoryItr->second.find(node->file.extension);
        if (extensionItr == std::end(categoryItr->second)) {
            return std::nullopt;
        }

        return extensionItr->second;
    }

    const std::filesystem::path& NodePainter::GetColoringFilePath() const
    {
        return m_fileColorMapPath;
    }
} // namespace Settings
