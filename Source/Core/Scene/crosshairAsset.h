#ifndef CROSSHAIRASSET_H
#define CROSSHAIRASSET_H

#include "lineAsset.h"

namespace Asset
{
   /**
    * @brief A simple crosshair overlay that's rendered over the visualization.
    */
   class Crosshair final : public Line
   {
   public:

      /**
       * @copydoc Asset::Base::Base()
       */
      Crosshair(
         const Settings::Manager& settings,
         QOpenGLExtraFunctions& openGL);

      /**
       * @copydoc Asset::Base::Render()
       */
      void Render(
         const Camera& camera,
         const std::vector<Light>& lights) override;

      /**
       * @brief Loads the necessary vertex and color data into the graphics buffers so as to show
       * the crosshair.
       *
       * @param[in] canvasCenter    The center of the canvas, which is where the crosshair is to
       *                            be overlaid.
       */
      void SetCrosshairLocation(const QPoint& canvasCenter);

   private:

      static constexpr wchar_t AssetName[] = L"Crosshair";
   };
}

#endif // CROSSHAIRASSET_H
