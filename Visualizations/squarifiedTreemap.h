#ifndef SQUARIFIEDTREEMAP_H
#define SQUARIFIEDTREEMAP_H

#include "visualization.h"

class SquarifiedTreeMap : public Visualization
{
   public:
      SquarifiedTreeMap(const VisualizationParameters& parameters);

      void Parse(const std::shared_ptr<Tree<VizNode>>& theTree) override;
};

#endif // SQUARIFIEDTREEMAP_H
