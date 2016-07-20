#ifndef SQUARIFIEDTREEMAP_H
#define SQUARIFIEDTREEMAP_H

#include "visualization.h"

/**
 * @brief The SquarifiedTreeMap class represents the squarified treemap visualization.
 */
class SquarifiedTreeMap : public VisualizationModel
{
   public:

      SquarifiedTreeMap(const VisualizationParameters& parameters);

      void Parse(const std::shared_ptr<Tree<VizNode>>& theTree) override;
};

#endif // SQUARIFIEDTREEMAP_H
