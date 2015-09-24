#ifndef GRIDASSET_H
#define GRIDASSET_H

#include "sceneAsset.h"

/**
 * @brief The GridAsset class implements the setup and rendering logic for the yellow grid that
 * appears under the visualization.
 */
class GridAsset : public SceneAsset
{
   public:
      explicit GridAsset();

      virtual bool LoadShaders() override;

      virtual bool PrepareVertexBuffers(const Camera& camera) override;
      virtual bool PrepareColorBuffers(const Camera& camera) override;

      virtual bool Render(const Camera& camera, const Light& light, bool isVizualizationLoaded,
                          const OptionsManager& settings) override;

      virtual bool Reload(const Camera& camera) override;
};

#endif // GRIDASSET_H
