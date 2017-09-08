#ifndef LIGHTMARKERASSET_H
#define LIGHTMARKERASSET_H

#include "lineAsset.h"

/**
 * @brief Series of markers to highlight the position of the lights.
 */
class LightMarkerAsset final : public LineAsset
{
   public:

      explicit LightMarkerAsset(QOpenGLExtraFunctions& device);

      bool Render(
         const Camera& camera,
         const std::vector<Light>& lights,
         const OptionsManager& settings) override;
};

#endif // LIGHTMARKERASSET_H
