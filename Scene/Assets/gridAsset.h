#ifndef GRIDASSET_H
#define GRIDASSET_H

#include "sceneAsset.h"

/**
 * @brief The GridAsset class implements the setup and rendering logic for the yellow grid that
 * appears under the visualization.
 */
class GridAsset : public SceneAsset
{
   public:
      GridAsset();
      ~GridAsset();

      virtual bool PrepareVertexBuffers(const Camera& camera) override;
      virtual bool PrepareColorBuffers(const Camera& camera) override;

      virtual bool Render(const Camera& camera) override;
};

#endif // GRIDASSET_H
