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

         explicit LightMarker(QOpenGLExtraFunctions& openGL);

         /**
          * @see Asset::Base::Render(...)
          */
         bool Render(
            const Camera& camera,
            const std::vector<Light>& lights,
            const OptionsManager& settings) override;
   };
}

#endif // LIGHTMARKERASSET_H
