#ifndef LIGHTMARKERASSET_H
#define LIGHTMARKERASSET_H

#include "lineAsset.h"

namespace Asset
{
   /**
    * @brief Series of markers to highlight the position of the lights.
    */
   class LightMarker final : public Line
   {
      public:

         /**
          * @see Asset::Base::Base(...)
          */
         LightMarker(
            QOpenGLExtraFunctions& openGL,
            bool isInitiallyVisible);
         /**
          * @see Asset::Base::Render(...)
          */
         void Render(
            const Camera& camera,
            const std::vector<Light>& lights,
            const Settings::Manager& settings) override;
   };
}

#endif // LIGHTMARKERASSET_H
