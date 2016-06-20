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

      bool InitializeUnitBlock();
      bool InitializeColors();
      bool InitializeBlockTransformations();

      QOpenGLBuffer m_unitBlockBuffer;
      QOpenGLBuffer m_blockTransformationBuffer;
      QOpenGLBuffer m_blockColorBuffer;

      QVector<QVector3D> m_unitBlockVertices;
      QVector<QVector3D> m_blockTransformations;
      QVector<QVector3D> m_blockColors;
};

#endif // VISUALIZATIONASSET_H
