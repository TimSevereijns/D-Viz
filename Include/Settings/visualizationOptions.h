#ifndef VISUALIZATIONOPTIONS_H
#define VISUALIZATIONOPTIONS_H

#include <string>

class VizBlock;

namespace Settings
{
    /**
     * @brief Visualization options that can be set to control which nodes are to be included
     * in the visualization.
     */
    class VisualizationOptions
    {
      public:
        /**
         * @brief Determines whether a node is visible in the visualization.
         *
         * @param[in] block             The node whose visibility is to be checked.
         *
         * @returns True if the node in question is visible to the end-user, given the current state
         * of the visualization options.
         */
        bool IsNodeVisible(const VizBlock& block) const noexcept;

        std::string rootDirectory;

        std::uint64_t minimumFileSize = 0;

        bool forceNewScan = true;
        bool onlyShowDirectories = false;
    };
} // namespace Settings

#endif // VISUALIZATIONOPTIONS_H
