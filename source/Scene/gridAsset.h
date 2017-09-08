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

      explicit GridAsset(QOpenGLExtraFunctions& device);

      bool Render(
         const Camera& camera,
         const std::vector<Light>& lights,
         const OptionsManager& settings) override;
};

#endif // GRIDASSET_H
