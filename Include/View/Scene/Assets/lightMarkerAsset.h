#ifndef LIGHTMARKERASSET_H
#define LIGHTMARKERASSET_H

#include "lineAsset.h"

namespace Assets
{
    /**
     * @brief Series of markers to highlight the position of the lights.
     */
    class LightMarker final : public Line
    {
      public:
        /**
         * @copydoc Asset::Base::Base()
         */
        LightMarker(const Controller& controller, QOpenGLExtraFunctions& openGL);

        /**
         * @copydoc Asset::Base::Render()
         */
        void Render(const Camera& camera, const std::vector<Light>& lights) override;
    };
} // namespace Assets

#endif // LIGHTMARKERASSET_H
