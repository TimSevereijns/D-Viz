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
         explicit Frustum(QOpenGLExtraFunctions& openGL);

         bool Render(
            const Camera& camera,
            const std::vector<Light>& lights,
            const OptionsManager& settings) override;

         void GenerateFrusta(const Camera& camera);
   };
}

#endif // FRUSTUMASSET_H
