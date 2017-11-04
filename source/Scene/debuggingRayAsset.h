#ifndef DEBUGGINGRAYASSET_H
#define DEBUGGINGRAYASSET_H

#include "lineAsset.h"

namespace Asset
{
   class DebuggingRay final : public Line
   {
      public:

         DebuggingRay(QOpenGLExtraFunctions& openGL);

         bool Render(
            const Camera& camera,
            const std::vector<Light>& lights,
            const OptionsManager& settings) override;
   };
}

#endif // DEBUGGINGRAYASSET_H
