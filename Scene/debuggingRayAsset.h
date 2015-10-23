#ifndef DEBUGGINGRAYASSET_H
#define DEBUGGINGRAYASSET_H

#include "lineAsset.h"

class DebuggingRayAsset : public LineAsset
{
   public:
      DebuggingRayAsset(GraphicsDevice& device);

      virtual bool Render(const Camera& camera, const Light& light,
         const OptionsManager& settings) override;

      virtual bool Reload(const Camera& camera) override;
};

#endif // DEBUGGINGRAYASSET_H
