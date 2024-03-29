#ifndef VISUALIZATIONASSET_H
#define VISUALIZATIONASSET_H

#include <memory>

#include "View/Scene/Assets/baseAsset.h"

#include <QOpenGLFramebufferObject>
#include <QOpenGLTexture>

template <typename DataType> class TreeNode;
class VizBlock;

struct BoundingBox;

namespace Assets
{
    class ShadowMapMetadata
    {
      public:
        explicit ShadowMapMetadata(std::unique_ptr<QOpenGLFramebufferObject> buffer) noexcept
            : framebuffer{ std::move(buffer) }
        {
        }

        std::unique_ptr<QOpenGLFramebufferObject> framebuffer;
        QMatrix4x4 projectionViewMatrix;
    };

    /**
     * @brief The Treemap class implements the functionality needed to render the treemap to the
     * OpenGL canvas.
     */
    class Treemap final : public AssetBase
    {
      public:
        /**
         * @copydoc Asset::Base::Base()
         */
        Treemap(const Controller& controller, QOpenGLExtraFunctions& openGL);

        /**
         * @copydoc Asset::Base::LoadShaders()
         */
        bool LoadShaders() override;

        /**
         * @copydoc Asset::Base::Initialize()
         */
        void Initialize() override;

        /**
         * @copydoc Asset::Base::Render()
         */
        void Render(const Camera& camera, const std::vector<Light>& lights) override;

        /**
         * @copydoc Asset::Base::Reload()
         */
        void Refresh() override;

        /**
         * @copydoc Asset::Base::IsAssetLoaded()
         */
        bool IsAssetLoaded() const override;

        /**
         * @brief Updates the vertex buffer object (VBO) to paint a particular node a particular
         * color.
         *
         * @param[in] node          The node to be located in the VBO and painted.
         * @param[in] color         The color to point the node.
         */
        void SetNodeColor(const Tree<VizBlock>::Node& node, const QVector3D& color);

        /**
         * @brief Loads the TreeMap nodes into the necessary graphics buffers.
         *
         * @note Passing the tree in as a const reference is a bit of a lie, since the nodes can
         * still be (and are) modifiable. Consider fixing this.
         *
         * @param[in] tree            The tree to pull the visualized TreeMap information from.
         *
         * @returns The number of blocks that have been loaded into the buffer.
         */
        std::uint32_t LoadBufferData(const Tree<VizBlock>& tree);

        /**
         * @brief Reloads the color buffer without touching the other buffers.
         *
         * @note Calling this function without populating the corresponding block transformation
         * and reference block buffers is likely to have unintended consequences.
         *
         * @param[in] tree            The tree to pull the visualized TreeMap information from.
         */
        void ReloadColorBufferData(const Tree<VizBlock>& tree);

        /**
         * @returns The number of blocks that are currently loaded into the visualization asset.
         */
        std::uint32_t GetBlockCount() const noexcept;

      private:
        void UpdateVBO(const Tree<VizBlock>::Node& node, const QVector3D& color);

        float ComputeWorldUnitsPerTexel(const BoundingBox& boundingBox);

        void InitializeShadowMachineryOnMainShader();
        void InitializeShadowMachineryOnShadowShader();

        void ComputeShadowMapProjectionViewMatrices(const Camera& camera);

        void RenderDepthMapPreview(std::size_t index);

        void RenderShadowPass(const Camera& camera);

        void RenderMainPass(const Camera& camera, const std::vector<Light>& lights);

        void ComputeAppropriateBlockColor(const Tree<VizBlock>::Node& node);

        void FindLargestDirectory(const Tree<VizBlock>& tree);

        void InitializeReferenceBlock();
        void InitializeColors();
        void InitializeBlockTransformations();
        void InitializeShadowMachinery();

        bool LoadTexturePreviewShaders();
        bool InitializeTexturePreviewer();

        int m_cascadeCount = 4;
        int m_shadowMapResolution = 4 * 1024;

        std::uint32_t m_blockCount = 0;

        std::uintmax_t m_largestDirectorySize = 0;

        double m_maxBoundingBoxDiagonal = 0.0;

        QOpenGLBuffer m_referenceBlockBuffer;
        QOpenGLBuffer m_blockTransformationBuffer;
        QOpenGLBuffer m_blockColorBuffer;
        QOpenGLBuffer m_texturePreviewVertexBuffer;

        QVector<QVector3D> m_referenceBlockVertices;
        QVector<QVector3D> m_blockColors;
        QVector<QMatrix4x4> m_blockTransformations;

        QOpenGLShaderProgram m_shadowMapShader;
        QOpenGLShaderProgram m_texturePreviewShader;

        std::vector<ShadowMapMetadata> m_shadowMaps;

        const Controller& m_controller;
    };
} // namespace Assets

#endif // VISUALIZATIONASSET_H
