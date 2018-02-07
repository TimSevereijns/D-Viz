#ifndef ORIGINMARKERASSET_H
#define ORIGINMARKERASSET_H

#include "lineAsset.h"

namespace Asset
{
   /**
    * @brief The Origin Marker class is meant to highlight the origin of the coordinate system
    * in the scene.
    */
   class OriginMarker final : public Line
   {
      public:

         /**
          * @see Asset::Base::Base(...)
          */
         OriginMarker(
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

#endif // ORIGINMARKERASSET_H
