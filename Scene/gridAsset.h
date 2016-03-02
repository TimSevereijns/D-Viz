#ifndef GRIDASSET_H
#define GRIDASSET_H

#include "lineAsset.h"

/**
 * @brief The GridAsset class implements the setup and rendering logic for the yellow grid that
 * appears under the visualization.
 */
class GridAsset : public LineAsset
{
   public:
      explicit GridAsset(GraphicsDevice& device);

      virtual bool Render(const Camera& camera, const std::vector<Light>& lights,
         const OptionsManager& settings) override;

      virtual bool Reload(const Camera& camera) override;
};

#endif // GRIDASSET_H
