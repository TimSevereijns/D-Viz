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

      // @todo Pass in the visualization and then call the init functions to load the transformation
      // matrices and the block colors.
      bool Initialize() override;

      bool LoadBufferData(const Tree<VizNode>& tree);

      bool Render(
         const Camera& camera,
         const std::vector<Light>& lights,
         const OptionsManager& settings) override;

      bool Reload() override;

      void UpdateVBO(
         const TreeNode<VizNode>& node,
         UpdateAction action,
         const VisualizationParameters& options) override;

      bool IsAssetLoaded() const override;

   private:

      bool InitializeUnitBlock();
      bool InitializeColors();
      bool InitializeBlockTransformations();

      QOpenGLBuffer m_referenceBlockBuffer;
      QOpenGLBuffer m_blockTransformationBuffer;
      QOpenGLBuffer m_blockColorBuffer;

      QVector<QVector3D> m_referenceBlockVertices;
      QVector<QMatrix4x4> m_blockTransformations;
      QVector<QVector3D> m_blockColors;
};

#endif // VISUALIZATIONASSET_H
