#ifndef VISUALIZATIONASSET_H
#define VISUALIZATIONASSET_H

#include "sceneAsset.h"

struct VizNode;
template<typename DataType> class TreeNode;

/**
 * @brief The VisualizationAsset class implements the functionality needed to represent the
 * main visualization scene asset.
 */
class VisualizationAsset : public SceneAsset
{
   public:
      VisualizationAsset(GraphicsDevice& device);

      bool LoadShaders() override;

      bool Initialize(const Camera& camera) override;

      bool Render(
         const Camera& camera,
         const std::vector<Light>& lights,
         const OptionsManager& settings) override;

      bool Reload(const Camera& camera) override;

      void UpdateVBO(
         const TreeNode<VizNode>& node,
         UpdateAction action,
         const VisualizationParameters& options) override;

   private:

      bool InitializeVertexBuffers(const Camera& camera);
      bool InitializeColorBuffers(const Camera& camera);

      QVector<QVector3D> m_blockPositions;
      QVector<QVector3D> m_blockDimensions;
};

#endif // VISUALIZATIONASSET_H
