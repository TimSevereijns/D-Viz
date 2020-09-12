#ifndef NODEPAINTER_H
#define NODEPAINTER_H

#include <algorithm>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>

// @note Pull this header in after the STL headers to avoid a weird compiler error related to
// Qt and a GCC macro in the STL library.
#include "settings.h"

#include <QObject>
#include <QVector3D>

struct VizBlock;

namespace Settings
{
    using ColorMap = std::unordered_map<std::wstring, std::unordered_map<std::wstring, QVector3D>>;

    class NodePainter : public QObject
    {
        Q_OBJECT

      public:
        NodePainter(const std::filesystem::path& colorFile = DefaultColoringFilePath());

        /**
         * @returns The map that associates colors with file extensions.
         */
        const ColorMap& GetFileColorMap() const;

        /**
         * @returns The currently active file extension coloring scheme.
         */
        const std::wstring& GetActiveColorScheme() const;

        /**
         * @brief Sets the current color scheme.
         */
        void SetColorScheme(std::wstring_view scheme);

        /**
         * @brief Determines the appropriate color for the file based on the user-configurable color
         * set in the color.json file.
         *
         * @param[in] extension     The extension to be painted.
         *
         * @returns The appropriate color found in the color map.
         */
        std::optional<QVector3D> DetermineColorFromExtension(std::wstring_view extension) const;

        /**
         * @returns The full path to the JSON file that contains the color mapping.
         */
        static std::filesystem::path DefaultColoringFilePath()
        {
            return std::filesystem::current_path().append(L"colors.json");
        }

      private:
        JsonDocument CreatePreferencesDocument();

        ColorMap m_colorMap;

        JsonDocument m_fileColorMapDocument;

        std::wstring m_colorScheme{ L"Default" };

        std::filesystem::path m_fileColorMapPath;
    };

} // namespace Settings

#endif // NODEPAINTER_H
