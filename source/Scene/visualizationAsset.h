#ifndef VISUALIZATIONASSET_H
#define VISUALIZATIONASSET_H

#include <memory>

#include "sceneAsset.h"
#include "texturePreviewAsset.h"

#include "../Utilities/colorGradient.hpp"

#include <QOpenGLFrameBufferObject>
#include <QOpenGLTexture>

struct VizNode;
template<typename DataType> class TreeNode;

/**
 * @brief The VisualizationAsset class implements the functionality needed to represent the
 * main visualization scene asset.
 */
class VisualizationAsset final : public SceneAsset
{
   public:

      VisualizationAsset(GraphicsDevice& device);

      bool LoadShaders() override;

      bool Initialize() override;

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

      /**
       * @brief Loads the TreeMap nodes into the necessary graphics buffers.
       *
       * @todo Passing the tree in as a const reference is a bit of a lie, since the nodes can still
       * be (and are) modifiable. Consider fixing this.
       *
       * @param[in] tree            The tree to pull the visualized TreeMap information from.
       * @param[in] parameters      The current visualization parameters that will govern what nodes
       *                            will be visualized.
       *
       * @returns The number of blocks that have been loaded into the buffer.
       */
      std::uint32_t LoadBufferData(
         const Tree<VizNode>& tree,
         const VisualizationParameters& parameters);

      /**
       * @returns The number of blocks that are currently loaded into the visualization asset.
       */
      std::uint32_t GetBlockCount() const;

   private:

      GLuint m_shadowMapTextureID{ 0 };
      GLuint m_shadowMapFrameBufferID{ 0 };

      TexturePreviewAsset* m_previewer;

      bool RenderShadowPass(const Camera& camera);

      bool RenderMainPass(
         const Camera& camera,
         const std::vector<Light>& lights,
         const OptionsManager& settings);

      QVector3D ComputeGradientColor(const TreeNode<VizNode>& node);

      void FindLargestDirectory(const Tree<VizNode>& tree);

      ColorGradient m_directoryColorGradient;

      std::uint32_t m_blockCount{ 0 };
      std::uintmax_t m_largestDirectorySize{ 0 };

      bool InitializeReferenceBlock();
      bool InitializeColors();
      bool InitializeBlockTransformations();
      bool InitializeShadowMachinery();

      QOpenGLBuffer m_referenceBlockBuffer;
      QOpenGLBuffer m_blockTransformationBuffer;
      QOpenGLBuffer m_blockColorBuffer;

      QVector<QVector3D> m_referenceBlockVertices;
      QVector<QVector3D> m_blockColors;
      QVector<QMatrix4x4> m_blockTransformations;

      QOpenGLShaderProgram m_shadowShader;

      static constexpr int SHADOW_MAP_WIDTH{ 4096 };
      static constexpr int SHADOW_MAP_HEIGHT{ 4096 };

      QMatrix4x4 m_lightSpaceTransformationMatrix;
};

#endif // VISUALIZATIONASSET_H
