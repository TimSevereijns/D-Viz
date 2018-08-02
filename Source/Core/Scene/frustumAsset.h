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
      void GenerateFrusta(const Camera& camera);

   private:

      static constexpr wchar_t AssetName[] = L"Frustum";
   };
}

#endif // FRUSTUMASSET_H
