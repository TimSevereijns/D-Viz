#include "Settings/visualizationParameters.h"

#include "Model/vizBlock.h"

namespace Settings
{
    bool VisualizationParameters::IsNodeVisible(const VizBlock& block) const noexcept
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
