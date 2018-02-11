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
            QOpenGLExtraFunctions& openGL);

         void Render(
            const Camera& camera,
            const std::vector<Light>& lights) override;

         void GenerateFrusta(const Camera& camera);

      private:

         static constexpr wchar_t AssetName[] = L"Frustum";
   };
}

#endif // FRUSTUMASSET_H
