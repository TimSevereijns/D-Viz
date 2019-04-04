#ifndef FRUSTUMASSET_H
#define FRUSTUMASSET_H

#include "lineAsset.h"

#include <string_view>

namespace Assets
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
        Frustum(const Settings::Manager& settings, QOpenGLExtraFunctions& openGL);

        /**
         * @copydoc Asset::Base::Render()
         */
        void Render(const Camera& camera, const std::vector<Light>& lights) override;

        /**
         * @brief GenerateFrusta
         *
         * @param camera
         */
        void GenerateFrusta(const Camera& camera);

      private:
        static constexpr std::wstring_view AssetName{ L"Frustum" };
    };
} // namespace Asset

#endif // FRUSTUMASSET_H