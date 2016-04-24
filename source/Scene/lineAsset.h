#ifndef LINEASSET_H
#define LINEASSET_H

#include "sceneAsset.h"

/**
 * @brief The LineAsset class
 */
class LineAsset : public SceneAsset
{
   public:
      explicit LineAsset(GraphicsDevice& device);

      bool LoadShaders() override;

      bool PrepareVertexBuffers(const Camera& camera) override;
      bool PrepareColorBuffers(const Camera& camera) override;

      bool Render(
         const Camera& camera,
         const std::vector<Light>& light,
         const OptionsManager& settings) override;

      bool Reload(const Camera& camera) override;
};

#endif // LINEASSET_H
