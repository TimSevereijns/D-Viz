#ifndef NODESELECTIONCROSSHAIR_H
#define NODESELECTIONCROSSHAIR_H

#include "lineAsset.h"

/**
 * @brief A simple crosshair overlay that's rendered over the visualization.
 */
class CrosshairAsset final : public LineAsset
{
   public:

      CrosshairAsset(QOpenGLExtraFunctions& device);

      bool Render(
         const Camera& camera,
         const std::vector<Light>& lights,
         const OptionsManager& settings) override;

      /**
       * @brief Loads the necessary vertex and color data into the graphics buffers so as to show
       * the crosshair.
       *
       * @param[in] canvasCenter    The center of the canvas, which is where the crosshair is to be
       *                            overlaid.
       */
      void SetCrosshairLocation(const QPoint& canvasCenter);
};

#endif // NODESELECTIONCROSSHAIR_H
