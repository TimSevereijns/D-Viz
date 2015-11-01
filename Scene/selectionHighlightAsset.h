#ifndef SELECTIONHIGHLIGHTASSET_H
#define SELECTIONHIGHLIGHTASSET_H

#include "lineAsset.h"

class SelectionHighlightAsset : public LineAsset
{
   public:
      explicit SelectionHighlightAsset(GraphicsDevice& device);

      virtual bool Render(const Camera& camera, const Light& light,
         const OptionsManager& settings) override;
};

#endif // SELECTIONHIGHLIGHTASSET_H
