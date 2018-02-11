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
            const Settings::Manager& settings,
            QOpenGLExtraFunctions& openGL);

         /**
          * @see Asset::Base::Render(...)
          */
         void Render(
            const Camera& camera,
            const std::vector<Light>& lights) override;

      private:

         static constexpr wchar_t AssetName[] = L"OriginMarker";
   };
}

#endif // ORIGINMARKERASSET_H
