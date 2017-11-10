#ifndef DEBUGGINGRAYASSET_H
#define DEBUGGINGRAYASSET_H

#include "lineAsset.h"

namespace Asset
{
   class DebuggingRay final : public Line
   {
      public:

         explicit DebuggingRay(QOpenGLExtraFunctions& openGL);

         /**
          * @see Asset::Base::Render(...)
          */
         bool Render(
            const Camera& camera,
            const std::vector<Light>& lights,
            const Settings::Manager& settings) override;
   };
}

#endif // DEBUGGINGRAYASSET_H
