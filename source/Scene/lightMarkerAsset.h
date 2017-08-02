#ifndef LIGHTMARKERASSET_H
#define LIGHTMARKERASSET_H

#include "lineAsset.h"

/**
 * @brief Simple marker to highlight the position of the lights.
 */
class LightMarkerAsset : public LineAsset
{
   public:

      explicit LightMarkerAsset(GraphicsDevice& device);

      bool Render(
         const Camera& camera,
         const std::vector<Light>& lights,
         const OptionsManager& settings) final override;

      bool Reload() override;
};

#endif // LIGHTMARKERASSET_H