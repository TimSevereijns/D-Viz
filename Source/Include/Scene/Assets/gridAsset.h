#ifndef GRIDASSET_H
#define GRIDASSET_H

#include "lineAsset.h"

namespace Assets
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
        Grid(const Controller& controller, QOpenGLExtraFunctions& openGL);

        /**
         * @copydoc Asset::Base::Render()
         */
        void Render(const Camera& camera, const std::vector<Light>& lights) override;
    };
} // namespace Assets

#endif // GRIDASSET_H
