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
            const Settings::Manager& settings,
            QOpenGLExtraFunctions& openGL,
            bool isInitiallyVisible);

         /**
          * @see Asset::Base::Render(...)
          */
         void Render(
            const Camera& camera,
            const std::vector<Light>& lights) override;
   };
}

#endif // DEBUGGINGRAYASSET_H
