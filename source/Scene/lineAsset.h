#ifndef LINEASSET_H
#define LINEASSET_H

#include "sceneAsset.h"

/**
 * @brief The LineAsset class
 */
class LineAsset : public SceneAsset
{
   public:

      explicit LineAsset(QOpenGLExtraFunctions& device);

      bool LoadShaders() override;

      bool Initialize() override;

      bool Render(
         const Camera& camera,
         const std::vector<Light>& light,
         const OptionsManager& settings) override;

      bool Reload() override;

   private:

      bool InitializeVertexBuffers();

      bool InitializeColorBuffers();
};

#endif // LINEASSET_H
