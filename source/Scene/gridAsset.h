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

         explicit Grid(QOpenGLExtraFunctions& openGL);

         /**
          * @see Asset::Base::Render(...)
          */
         bool Render(
            const Camera& camera,
            const std::vector<Light>& lights,
            const OptionsManager& settings) override;
   };
}

#endif // GRIDASSET_H
