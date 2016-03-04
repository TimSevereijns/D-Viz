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

      virtual bool LoadShaders() override;

      virtual bool PrepareVertexBuffers(const Camera& camera) override;
      virtual bool PrepareColorBuffers(const Camera& camera) override;

      virtual bool Render(
         const Camera& camera,
         const std::vector<Light>& light,
         const OptionsManager& settings) override;

      virtual bool Reload(const Camera& camera) override;
};

#endif // LINEASSET_H
