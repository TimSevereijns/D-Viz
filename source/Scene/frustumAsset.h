#ifndef FRUSTUMASSET_H
#define FRUSTUMASSET_H

#include "lineAsset.h"

namespace Asset
{
   /**
    * @brief The frustumAsset class
    */
   class Frustum final : public Line
   {
      public:

         explicit Frustum(
            const Settings::Manager& settings,
            QOpenGLExtraFunctions& openGL,
            bool isInitiallyVisible);

         void Render(
            const Camera& camera,
            const std::vector<Light>& lights) override;

         void GenerateFrusta(const Camera& camera);
   };
}

#endif // FRUSTUMASSET_H
