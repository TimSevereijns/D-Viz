#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include "preferencesMap.hpp"
#include "settings.h"

#include <optional>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <variant>

#include <Tree/Tree.hpp>

#include <QObject>
#include <QVector3D>

struct VizBlock;

namespace Settings
{
    using ColorMap = std::unordered_map<std::wstring, std::unordered_map<std::wstring, QVector3D>>;

    /**
     * @brief The central class responsible for run-time settings.
     */
    class PersistentSettings final : public QObject
    {
        Q_OBJECT

      public:
        PersistentSettings(
            const std::filesystem::path& colorFile, const std::filesystem::path& preferencesFile);

        /**
         * @returns The map that associates colors with file extensions.
         */
        const ColorMap& GetFileColorMap() const;

        /**
         * @returns The preference map.
         */
        const PreferencesMap& GetPreferenceMap() const;

        /**
         * @returns The currently active file extension coloring scheme.
         */
        const std::wstring& GetActiveColorScheme() const;

        /**
         * @brief Sets the current color scheme.
         */
        void SetColorScheme(const std::wstring& scheme);

        /**
         * @returns True if shadow cascade splits should be visualized.
         */
        bool ShouldShowCascadeSplits() const;

        /**
         * @brief Passing in `true` will render the cascade splits overlay.
         */
        void SetShowCascadeSplits(bool isEnabled);

        /**
         * @returns True if shadow rendering is enabled.
         */
        bool ShouldRenderShadows() const;

        /**
         * @brief Passing in `true` will enable the rendering of shadows.
         */
        void SetShowShadows(bool isEnabled);

        /**
         * @returns True if the file system monitor is enabled;
         */
        bool ShouldMonitorFileSystem() const;

        /**
         * @brief Determines the appropriate color for the file based on the user-configurable color
         * set in the color.json file.
         *
         * @param[in] node               The node whose color needs to be restored.
         *
         * @returns The appropriate color found in the color map.
         */
        std::optional<QVector3D>
        DetermineColorFromExtension(const Tree<VizBlock>::Node& node) const;

        /**
         * @brief Handles toggling of whether the file system should be monitored for changes.
         *
         * @param[in] isEnabled       Pass in true to enable monitoring.
         */
        void OnMonitoringOptionToggled(bool isEnabled);

        /**
         * @brief Saves preferences to a JSON file on disk.
         *
         * @param[in] property        The name of the property to be saved or modified.
         * @param[in] value           The value to be associated with the property name.
         *
         * @returns True if the save action succeeded.
         */
        template <typename PropertyValueType>
        bool SaveSettingToDisk(std::wstring_view property, const PropertyValueType& value)
        {
            if (m_preferencesDocument.HasMember(property.data())) {
                m_preferencesDocument[property.data()] = value;
            } else {
                auto& allocator = m_preferencesDocument.GetAllocator();

                rapidjson::GenericValue<rapidjson::UTF16<wchar_t>> key{ property.data(),
                                                                        allocator };
                m_preferencesDocument.AddMember(key.Move(), value, allocator);
            }

            return Settings::SaveToDisk(m_preferencesDocument, m_preferencesPath);
        }

        /**
         * @returns The full path to the JSON file that contains the color mapping.
         */
        static std::filesystem::path GetColorJsonPath()
        {
            return std::filesystem::current_path().append(L"colors.json");
        }

        /**
         * @returns The full path to the JSON file that contains the user preferences.
         */
        static std::filesystem::path GetPreferencesJsonPath()
        {
            return std::filesystem::current_path().append(L"preferences.json");
        }

      private:
        JsonDocument CreatePreferencesDocument();

        bool m_showCascadeSplits{ false };
        bool m_shouldShowShadows{ true };
        bool m_shouldMonitorFileSystem{ true };

        JsonDocument m_fileColorMapDocument;
        JsonDocument m_preferencesDocument;

        std::filesystem::path m_preferencesPath;
        std::filesystem::path m_fileColorMapPath;

        ColorMap m_colorMap;
        PreferencesMap m_preferencesMap;

        std::wstring m_colorScheme{ L"Default" };
    };
} // namespace Settings

#endif // SETTINGSMANAGER_H
