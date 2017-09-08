#ifndef FRUSTUMASSET_H
#define FRUSTUMASSET_H

#include "lineAsset.h"

/**
 * @brief The frustumAsset class
 */
class FrustumAsset final : public LineAsset
{
   public:
      explicit FrustumAsset(QOpenGLExtraFunctions& device);

      bool Render(
         const Camera& camera,
         const std::vector<Light>& lights,
         const OptionsManager& settings) override;

      void GenerateFrusta(const Camera& camera);
};

#endif // FRUSTUMASSET_H
