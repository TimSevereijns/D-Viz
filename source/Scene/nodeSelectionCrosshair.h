#ifndef NODESELECTIONCROSSHAIR_H
#define NODESELECTIONCROSSHAIR_H

#include "lineAsset.h"

#include <QPoint>

class NodeSelectionCrosshair: public LineAsset
{
   public:
      NodeSelectionCrosshair(GraphicsDevice& device);

      void Show(const Camera& camera);

      void Hide();

      bool Render(
         const Camera& camera,
         const std::vector<Light>& lights,
         const OptionsManager& settings) override;
};

#endif // NODESELECTIONCROSSHAIR_H
