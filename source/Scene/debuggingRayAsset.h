#ifndef DEBUGGINGRAYASSET_H
#define DEBUGGINGRAYASSET_H

#include "lineAsset.h"

namespace Asset
{
   class DebuggingRay final : public Line
   {
      public:

         /**
          * @see Asset::Base::Base(...)
          */
         DebuggingRay(
            QOpenGLExtraFunctions& openGL,
            bool isInitiallyVisible);

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
