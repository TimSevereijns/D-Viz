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
       * @copydoc Asset::Base::Base()
       */
      LightMarker(
         const Settings::Manager& settings,
         QOpenGLExtraFunctions& openGL);
      /**
       * @copydoc Asset::Base::Render()
       */
      void Render(
         const Camera& camera,
         const std::vector<Light>& lights) override;

   private:

      static constexpr wchar_t AssetName[] = L"LightMarker";
   };
}

#endif // LIGHTMARKERASSET_H
