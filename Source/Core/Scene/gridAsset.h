#ifndef GRIDASSET_H
#define GRIDASSET_H

#include "lineAsset.h"

#include <string_view>

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
         * @copydoc Asset::Base::Base()
         */
        Grid(const Settings::Manager& settings, QOpenGLExtraFunctions& openGL);

        /**
         * @copydoc Asset::Base::Render()
         */
        void Render(const Camera& camera, const std::vector<Light>& lights) override;

      private:
        static constexpr std::wstring_view AssetName{ L"Grid" };
    };
} // namespace Asset

#endif // GRIDASSET_H
