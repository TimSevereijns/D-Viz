#ifndef NODESELECTIONCROSSHAIR_H
#define NODESELECTIONCROSSHAIR_H

#include "lineAsset.h"

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
       * @param[in] camera          The camera onto which the crosshair is to be overlaid.
       */
      void Show(const Camera& camera);

      /**
       * @brief Clears the buffers, thereby "hiding" the crosshair.
       */
      void Hide();
};

#endif // NODESELECTIONCROSSHAIR_H
