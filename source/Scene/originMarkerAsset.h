#ifndef ORIGINMARKERASSET_H
#define ORIGINMARKERASSET_H

#include "lineAsset.h"

class OriginMarkerAsset : public LineAsset
{
   public:

      explicit OriginMarkerAsset(QOpenGLExtraFunctions& graphicsContext);

      bool Render(
         const Camera& camera,
         const std::vector<Light>& lights,
         const OptionsManager& settings) override;
};

#endif // ORIGINMARKERASSET_H
