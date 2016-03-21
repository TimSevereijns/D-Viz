#ifndef NODESELECTIONCROSSHAIR_H
#define NODESELECTIONCROSSHAIR_H

#include "lineAsset.h"

#include <QPoint>

class NodeSelectionCrosshair: public LineAsset
{
   public:
      NodeSelectionCrosshair(GraphicsDevice& device);

      void ShowCrosshairPosition(const QPoint& crosshairCenter);

      void HideCrosshair();

      bool Render(
         const Camera& camera,
         const std::vector<Light>& lights,
         const OptionsManager& settings) override;
};

#endif // NODESELECTIONCROSSHAIR_H
