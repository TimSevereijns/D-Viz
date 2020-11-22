#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <algorithm>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>

// @note Pull this header in after the STL headers to avoid a weird compiler error related to
// Qt and a GCC macro in the STL library.
#include "settings.h"

#include <Tree/Tree.hpp>

#include <QObject>
#include <QVector3D>

class VizBlock;

namespace Settings
{
    /**
     * @brief The central class responsible for run-time settings.
     */
    class PersistentSettings final : public QObject
    {
        Q_OBJECT

      public:
        explicit PersistentSettings(
            const std::filesystem::path& preferencesFile = DefaultPreferencesFilePath());

        /**
         * @returns True if shadow cascade splits should be visualized.
         */
        bool ShouldRenderCascadeSplits() const;

        /**
         * @brief Passing in `true` will render the cascade splits overlay.
         */
        void RenderCascadeSplits(bool isEnabled);

        /**
         * @returns True if shadow rendering is enabled.
         */
        bool ShouldRenderShadows() const;

        /**
         * @brief Passing in `true` will enable the rendering of shadows.
         */
        void RenderShadows(bool isEnabled);

        /**
         * @returns True if the file system monitor is enabled;
         */
        bool ShouldMonitorFileSystem() const;

        /**
         * @brief Handles toggling of whether the filesystem should be monitored for changes.
         *
         * @param[in] isEnabled       Pass in true to enable monitoring.
         */
        void MonitorFileSystem(bool isEnabled);

        /**
         * @returns True if origin of the coordinate system should be visualized.
         */
        bool ShouldRenderOrigin() const;

        /**
         * @brief Toggles the display of an coordinate system origin marker.
         *
         * @param[in] isEnabled       Pass in true to enable visualization.
         */
        void RenderOrigin(bool isEnabled);

        /**
         * @returns True if the grid should be rendered.
         */
        bool ShouldRenderGrid() const;

        /**
         * @brief Toggles the display of the grid.
         *
         * @param[in] isEnabled       Pass in true to enable grid.
         */
        void RenderGrid(bool isEnabled);

        /**
         * @returns True if the location of the lights should be marked. Useful for debugging.
         */
        bool ShouldRenderLightMarkers() const;

        /**
         * @brief Toggles the display of light markers.
         *
         * @param[in] isEnabled       Pass in true to display markers.
         */
        void RenderLightMarkers(bool isEnabled);

        /**
         * @returns True if a static view frustum should be show. Useful for debugging.
         */
        bool ShouldRenderFrusta() const;

        /**
         * @brief Toggles the display of debugging frusta.
         *
         * @param[in] isEnabled       Pass in true to enable frusta.
         */
        void RenderFrusta(bool isEnabled);

        /**
         * @returns The number of cascade counts, clamped between 1 and 4, inclusive.
         */
        int GetShadowMapCascadeCount() const;

        /**
         * @brief Sets the number of cascade splits to use.
         *
         * @param[in] count         A value between 1 and 4, inclusive.
         */
        void SetShadowMapCascadeCount(int count);

        /**
         * @returns The quality (i.e., resolution) of the shadow map, clamped between 1 and 4,
         * inclusive.
         *
         * 1 is equivalent to 1024 by 1024 pixels, while 4 is equivalent to 4096 by 4096 pixels.
         */
        int GetShadowMapQuality() const;

        /**
         * @brief Sets the shadow map quality.
         *
         * 1 is equivalent to 1024 by 1024 pixels, while 4 is equivalent to 4096 by 4096 pixels.
         *
         * @param[in] quality       A value between 1 and 4, inclusive.
         */
        void SetShadowMapQuality(int quality);

        /**
         * @returns True if the debugging menu should be shown.
         */
        bool ShouldShowDebuggingMenu() const;

        /**
         * @brief Toggles the use of the dark theme.
         */
        void UseDarkMode(bool isEnabled);

        /**
         * @return True if the dark theme should be used.
         */
        bool ShouldUseDarkMode() const;

        /**
         * @brief Saves all settings to disk.
         *
         * @returns True if the operation succeeded.
         */
        bool SaveAllPreferencesToDisk();

        /**
         * @returns The current preferences path.
         */
        const std::filesystem::path& GetPreferencesFilePath() const;

        /**
         * @returns The full path to the JSON file that contains the user preferences.
         */
        static std::filesystem::path DefaultPreferencesFilePath()
        {
            return std::filesystem::current_path().append(L"preferences.json");
        }

      private:
        JsonDocument CreatePreferencesDocument();

        JsonDocument m_preferencesDocument;

        std::filesystem::path m_preferencesPath;
    };
} // namespace Settings

#endif // SETTINGSMANAGER_H
