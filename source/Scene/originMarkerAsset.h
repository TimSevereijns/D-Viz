#ifndef ORIGINMARKERASSET_H
#define ORIGINMARKERASSET_H

#include "lineAsset.h"

namespace Asset
{
   class OriginMarker final : public Line
   {
      public:

         explicit OriginMarker(QOpenGLExtraFunctions& openGL);

         bool Render(
            const Camera& camera,
            const std::vector<Light>& lights,
            const OptionsManager& settings) override;
   };
}

#endif // ORIGINMARKERASSET_H
