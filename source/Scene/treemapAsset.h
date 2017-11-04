#ifndef VISUALIZATIONASSET_H
#define VISUALIZATIONASSET_H

#include <memory>

#include "baseAsset.h"
#include "texturePreviewAsset.h"

#include "../Utilities/colorGradient.hpp"

#include <QOpenGLFrameBufferObject>
#include <QOpenGLTexture>

struct VizFile;

template<typename DataType>
class TreeNode;

namespace Asset
{
   /**
    * @brief The VisualizationAsset class implements the functionality needed to represent the
    * main visualization scene asset.
    */
   class Treemap final : public Base
   {
      public:

         Treemap(QOpenGLExtraFunctions& openGL);

         bool LoadShaders() override;

         bool Initialize() override;

         bool Render(
            const Camera& camera,
            const std::vector<Light>& lights,
            const OptionsManager& settings) override;

         bool Reload() override;

         void UpdateVBO(
            const Tree<VizFile>::Node& node,
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
            const Tree<VizFile>& tree,
            const VisualizationParameters& parameters);

         /**
          * @returns The number of blocks that are currently loaded into the visualization asset.
          */
         std::uint32_t GetBlockCount() const;

      private:

         void RenderDepthMapPreview();

         void RenderShadowPass(const Camera& camera);

         void RenderMainPass(
            const Camera& camera,
            const std::vector<Light>& lights,
            const OptionsManager& settings);

         QVector3D ComputeGradientColor(const Tree<VizFile>::Node& node);

            void FindLargestDirectory(const Tree<VizFile>& tree);

         bool InitializeReferenceBlock();
         bool InitializeColors();
         bool InitializeBlockTransformations();
         bool InitializeShadowMachinery();

         bool LoadTexturePreviewShaders();
         bool InitializeTexturePreviewer();

         ColorGradient m_directoryColorGradient;

            std::uint32_t m_blockCount{ 0 };
            std::uintmax_t m_largestDirectorySize{ 0 };

         QOpenGLBuffer m_referenceBlockBuffer;
         QOpenGLBuffer m_blockTransformationBuffer;
         QOpenGLBuffer m_blockColorBuffer;
         QOpenGLBuffer m_texturePreviewVertexBuffer;

         QVector<QVector3D> m_referenceBlockVertices;
         QVector<QVector3D> m_blockColors;
         QVector<QMatrix4x4> m_blockTransformations;

         QOpenGLShaderProgram m_shadowMapShader;
         QOpenGLShaderProgram m_texturePreviewShader;

         Camera m_shadowCamera;

         static constexpr auto SHADOW_MAP_WIDTH{ 8192 };
         static constexpr auto SHADOW_MAP_HEIGHT{ 8192 };

         QOpenGLFramebufferObject m_shadowMapFrameBuffer
         {
            SHADOW_MAP_WIDTH,
            SHADOW_MAP_HEIGHT,
            QOpenGLFramebufferObject::Depth,
            GL_TEXTURE_2D,
            GL_R32F
         };

         QMatrix4x4 m_lightSpaceTransformationMatrix;
   };
}

#endif // VISUALIZATIONASSET_H
