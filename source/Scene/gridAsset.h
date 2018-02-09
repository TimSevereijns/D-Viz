#ifndef GRIDASSET_H
#define GRIDASSET_H

#include "lineAsset.h"

namespace Asset
{
   /**
    * @brief The GridAsset class implements the setup and rendering logic for the yellow grid that
    * appears under the visualization.
    */
   class Grid final : public Line
   {
      public:

         /**
          * @see Asset::Base::Base(...)
          */
         Grid(
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

#endif // GRIDASSET_H
