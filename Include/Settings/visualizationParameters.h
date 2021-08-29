#ifndef VISUALIZATIONPARAMETERS_H
#define VISUALIZATIONPARAMETERS_H

#include <string>

class VizBlock;

namespace Settings
{
    /**
     * @brief Visualization parameters that can be set to control which nodes are to be included
     * in the visualization.
     */
    class VisualizationParameters
    {
      public:
        /**
         * @brief Determines whether a node is visible in the visualization.
         *
         * @param[in] block             The node whose visibility is to be checked.
         *
         * @returns True if the node in question is visible to the end-user, given the current state
         * of the visualization parameters.
         */
        bool IsNodeVisible(const VizBlock& block) const noexcept;

        std::string rootDirectory;

        std::uint64_t minimumFileSize = 0;

        bool forceNewScan = true;
        bool onlyShowDirectories = false;
    };
} // namespace Settings

#endif // VISUALIZATIONPARAMETERS_H
