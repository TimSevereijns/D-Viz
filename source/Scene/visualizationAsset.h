#ifndef VISUALIZATIONASSET_H
#define VISUALIZATIONASSET_H

#include "sceneAsset.h"

#include "../Utilities/colorGradient.hpp"

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

      bool Initialize() override;

      std::uint32_t LoadBufferData(
         Tree<VizNode>& tree,
         const VisualizationParameters& parameters);

      std::uint32_t GetBlockCount() const;

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

      QVector3D ComputeGradientColor(const TreeNode<VizNode>& node);

      void FindLargestDirectory(const Tree<VizNode>& tree);

      ColorGradient m_directoryColorGradient;

      std::uint32_t m_blockCount{ 0 };
      std::uintmax_t m_largestDirectorySize{ 0 };

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
