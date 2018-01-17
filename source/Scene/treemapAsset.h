#ifndef VISUALIZATIONASSET_H
#define VISUALIZATIONASSET_H

#include <memory>

#include "baseAsset.h"

#include "../Utilities/colorGradient.hpp"

#include <QOpenGLFramebufferObject>
#include <QOpenGLTexture>

struct VizFile;

template<typename DataType>
class TreeNode;

namespace Asset
{
   /**
    * @brief The Treemap class implements the functionality needed to render the treemap to the
    * OpenGL canvas.
    */
   class Treemap final : public Base
   {
      public:

         constexpr static auto CASCADE_COUNT{ 3 };
         constexpr static auto SHADOW_MAP_WIDTH{ 1024 * 4 };
         constexpr static auto SHADOW_MAP_HEIGHT{ 1024 * 4 };

         /**
          * @see Asset::Base::Base(...)
          */
         Treemap(
            QOpenGLExtraFunctions& openGL,
            bool isInitiallyVisible);

         /**
          * @see Asset::Base::LoadShaders(...)
          */
         bool LoadShaders() override;

         /**
          * @see Asset::Base::Initialize(...)
          */
         bool Initialize() override;

         /**
          * @see Asset::Base::Render(...)
          */
         bool Render(
            const Camera& camera,
            const std::vector<Light>& lights,
            const Settings::Manager& settings) override;

         /**
          * @see Asset::Base::Reload(...)
          */
         bool Refresh() override;

         /**
          * @see Asset::Base::UpdateVBO(...)
          */
         void UpdateVBO(
            const Tree<VizFile>::Node& node,
            Asset::Event action,
            const Settings::Manager& settings) override;

         /**
          * @see Asset::Base::IsAssetLoaded(...)
          */
         bool IsAssetLoaded() const override;

         /**
          * @brief Loads the TreeMap nodes into the necessary graphics buffers.
          *
          * @note Passing the tree in as a const reference is a bit of a lie, since the nodes can
          * still be (and are) modifiable. Consider fixing this.
          *
          * @param[in] tree            The tree to pull the visualized TreeMap information from.
          * @param[in] settings        A reference to the settings manager so that the visualization
          *                            can take into account the current settings.
          *
          * @returns The number of blocks that have been loaded into the buffer.
          */
         std::uint32_t LoadBufferData(
            const Tree<VizFile>& tree,
            const Settings::Manager& settings);

         /**
          * @brief Reloads the color buffer without touching the other buffers.
          *
          * @note Calling this function without populating the corresponding block transformation
          * and reference block buffers is likely to have unintended consequences.
          *
          * @param[in] tree            The tree to pull the visualized TreeMap information from.
          * @param[in] settings        A reference to the settings manager so that the visualization
          *                            can take into account the current settings.
          */
         void ReloadColorBufferData(
            const Tree<VizFile>& tree,
            const Settings::Manager& settings);

         /**
          * @returns The number of blocks that are currently loaded into the visualization asset.
          */
         std::uint32_t GetBlockCount() const;

      private:

         void InitializeShadowMachineryOnMainShader();
         void InitializeShadowMachineryOnShadowShader();

         void ComputeShadowMapProjectionViewMatrices(const Camera& camera);

         void RenderDepthMapPreview(int index);

         void RenderShadowPass(const Camera& camera);

         void RenderMainPass(
            const Camera& camera,
            const std::vector<Light>& lights,
            const Settings::Manager& settings);

         QVector3D ComputeGradientColor(const Tree<VizFile>::Node& node);

         void ComputeAppropriateBlockColor(
            const Tree<VizFile>::Node& node,
            const Settings::Manager& settings);

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

         struct ShadowMapMetadata
         {
            ShadowMapMetadata(
               std::unique_ptr<QOpenGLFramebufferObject> buffer,
               QMatrix4x4 matrix,
               int location)
               :
               framebuffer{ std::move(buffer) },
               projectionViewMatrix{ std::move(matrix) },
               textureLocation{ location }
            {
            }

            std::unique_ptr<QOpenGLFramebufferObject> framebuffer;
            QMatrix4x4 projectionViewMatrix;
            int textureLocation;
         };

         std::vector<ShadowMapMetadata> m_shadowMaps;
   };
}

#endif // VISUALIZATIONASSET_H
