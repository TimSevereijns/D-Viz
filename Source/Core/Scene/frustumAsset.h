#ifndef FRUSTUMASSET_H
#define FRUSTUMASSET_H

#include "lineAsset.h"

#include <string_view>

namespace Asset
{
   /**
    * @brief The frustumAsset class
    */
   class Frustum final : public Line
   {
   public:

      /**
       * @copydoc Asset::Base::Base()
       */
      Frustum(
         const Settings::Manager& settings,
         QOpenGLExtraFunctions& openGL);

      /**
       * @copydoc Asset::Base::Render()
       */
      void Render(
         const Camera& camera,
         const std::vector<Light>& lights) override;

      /**
       * @brief GenerateFrusta
       *
       * @param camera
       */
      void GenerateFrusta(Camera camera);

   private:

      static constexpr std::wstring_view AssetName{ L"Frustum" };
   };
}

#endif // FRUSTUMASSET_H
