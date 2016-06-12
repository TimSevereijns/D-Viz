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

      bool Initialize(const Camera &camera) override;

      bool Render(
         const Camera& camera,
         const std::vector<Light>& light,
         const OptionsManager& settings) override;

      bool Reload(const Camera& camera) override;

   private:

      bool InitializeVertexBuffers(const Camera& camera);

      bool InitializeColorBuffers(const Camera& camera);

};

#endif // LINEASSET_H
