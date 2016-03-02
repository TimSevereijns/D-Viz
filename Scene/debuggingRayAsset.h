#ifndef DEBUGGINGRAYASSET_H
#define DEBUGGINGRAYASSET_H

#include "lineAsset.h"

class DebuggingRayAsset : public LineAsset
{
   public:
      DebuggingRayAsset(GraphicsDevice& device);

      virtual bool Render(const Camera& camera, const std::vector<Light>& lights,
         const OptionsManager& settings) override;
};

#endif // DEBUGGINGRAYASSET_H
