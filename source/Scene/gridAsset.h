#ifndef GRIDASSET_H
#define GRIDASSET_H

#include "lineAsset.h"

/**
 * @brief The GridAsset class implements the setup and rendering logic for the yellow grid that
 * appears under the visualization.
 */
class GridAsset final : public LineAsset
{
   public:

      explicit GridAsset(GraphicsDevice& device);

      bool Render(
         const Camera& camera,
         const std::vector<Light>& lights,
         const OptionsManager& settings) override;

      bool Reload() override;
};

#endif // GRIDASSET_H
