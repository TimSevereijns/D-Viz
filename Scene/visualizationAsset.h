#ifndef VISUALIZATIONASSET_H
#define VISUALIZATIONASSET_H

#include "sceneAsset.h"

/**
 * @brief The VisualizationAsset class implements the functionality needed to represent the
 * main visualization scene asset.
 */
class VisualizationAsset : public SceneAsset
{
   public:
      VisualizationAsset(GraphicsDevice& device);

      virtual bool LoadShaders() override;

      virtual bool PrepareVertexBuffers(const Camera& camera) override;
      virtual bool PrepareColorBuffers(const Camera& camera) override;

      virtual bool Render(const Camera& camera, const Light& light,
         const OptionsManager& settings) override;

      virtual bool Reload(const Camera& camera) override;
};

#endif // VISUALIZATIONASSET_H
