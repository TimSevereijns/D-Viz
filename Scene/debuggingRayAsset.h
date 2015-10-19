#ifndef DEBUGGINGRAYASSET_H
#define DEBUGGINGRAYASSET_H

#include "sceneAsset.h"

class DebuggingRayAsset : public SceneAsset
{
   public:
      DebuggingRayAsset(GraphicsDevice& device);

      virtual bool LoadShaders() override;

      virtual bool PrepareVertexBuffers(const Camera& camera) override;
      virtual bool PrepareColorBuffers(const Camera& camera) override;

      virtual bool Render(const Camera& camera, const Light& light,
         const OptionsManager& settings) override;

      virtual bool Reload(const Camera& camera) override;
};

#endif // DEBUGGINGRAYASSET_H
