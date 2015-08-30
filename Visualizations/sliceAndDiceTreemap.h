#ifndef SLICEANDDICETREEMAP_H
#define SLICEANDDICETREEMAP_H

#include "visualization.h"

class SliceAndDiceTreeMap : public Visualization
{
   public:
      SliceAndDiceTreeMap(const VisualizationParameters& parameters);

      void Parse(const std::shared_ptr<Tree<VizNode> >& theTree) override;
};

#endif // SLICEANDDICETREEMAP_H
