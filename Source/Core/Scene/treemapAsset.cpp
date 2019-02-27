#include "treemapAsset.h"

#include "../DataStructs/vizBlock.h"
#include "../Settings/settings.h"
#include "../Utilities/colorGradient.hpp"
#include "../Utilities/scopeExit.hpp"
#include "../Utilities/viewFrustum.hpp"
#include "../Visualizations/visualization.h"
#include "../constants.h"

#include <Tree/Tree.hpp>
#include <boost/optional.hpp>
#include <gsl/gsl_assert>
#include <spdlog/spdlog.h>

#include <cmath>
#include <iostream>
#include <vector>

#ifdef Q_OS_WIN
#undef near
#undef far
#endif

// @todo Consider hoisting this out to another file.
struct BoundingBox
{
    float left;
    float right;
    float bottom;
    float top;
    float near;
    float far;
};

namespace
{
    constexpr auto TEXTURE_PREVIEWER_VERTEX_ATTRIBUTE{ 0 };
    constexpr auto TEXTURE_PREVIEWER_TEXCOORD_ATTRIBUTE{ 1 };

    /**
     * @brief Calculates an Axis Aligned Bounding Box (AABB) for each of the frustum splits.
     *
     * @param[in] renderCamera       The main camera used to render the scene. Mainly use is to get
     *                               the aspect ratio of the outline correct.
     * @param[in] shadowViewMatrix   The projection-view matrix that represents the view of the
     *                               shadow casting light source.
     * @param[in] cascadeCount       The number of cascades.
     */
    auto ComputeFrustumSplitBoundingBoxes(
        const Camera& renderCamera, const QMatrix4x4& shadowViewMatrix, int cascadeCount)
    {
        Expects(cascadeCount >= 0);

        static const auto cascadeDistances = FrustumUtilities::GetCascadeDistances();

        std::vector<std::vector<QVector3D>> frusta;
        frusta.reserve(static_cast<std::size_t>(cascadeCount));

        auto mutableCamera = renderCamera;

        for (const auto& nearAndFarPlanes : cascadeDistances) {
            mutableCamera.SetNearPlane(nearAndFarPlanes.first);
            mutableCamera.SetFarPlane(nearAndFarPlanes.second);

            frusta.emplace_back(FrustumUtilities::ComputeFrustumCorners(mutableCamera));
        }

        std::vector<BoundingBox> boundingBoxes;
        boundingBoxes.reserve(static_cast<std::size_t>(cascadeCount));

        const auto worldToLight = shadowViewMatrix;

        for (const auto& frustum : frusta) {
            auto minX = std::numeric_limits<float>::max();
            auto maxX = std::numeric_limits<float>::lowest();

            auto minY = std::numeric_limits<float>::max();
            auto maxY = std::numeric_limits<float>::lowest();

            auto minZ = std::numeric_limits<float>::max();
            auto maxZ = std::numeric_limits<float>::lowest();

            // Compute bounding box in light space:
            for (const auto& vertex : frustum) {
                const auto mappedVertex = worldToLight.map(vertex);

                minX = std::min(minX, mappedVertex.x());
                maxX = std::max(maxX, mappedVertex.x());

                minY = std::min(minY, mappedVertex.y());
                maxY = std::max(maxY, mappedVertex.y());

                minZ = std::min(minZ, mappedVertex.z());
                maxZ = std::max(maxZ, mappedVertex.z());
            }

            auto boundingBox = BoundingBox{ /* left   = */ minX,
                                            /* right  = */ maxX,
                                            /* bottom = */ minY,
                                            /* top    = */ maxY,
                                            /* near   = */ maxZ,
                                            /* far    = */ minZ };

            boundingBoxes.emplace_back(boundingBox);
        }

        return boundingBoxes;
    }

    /**
     * @brief A helper function to set many of the shader variables needed for lighting.
     *
     * @param[in] lights             Vector of lights to be loaded into the shader program.
     * @param[in] settings           Additional scene rendering settings.
     * @param[out] shader            The shader program to load the light data into.
     */
    void SetUniformLights(
        const std::vector<Light>& lights, const Settings::Manager& settings,
        QOpenGLShaderProgram& shader)
    {
        for (auto index{ 0u }; index < lights.size(); ++index) {
            const auto indexAsString = std::to_string(index);

            const auto position = "allLights[" + indexAsString + "].position";
            shader.setUniformValue(position.data(), lights[index].position);

            const auto intensity = "allLights[" + indexAsString + "].intensity";
            shader.setUniformValue(intensity.data(), lights[index].intensity);

            const auto attenuation = "allLights[" + indexAsString + "].attenuation";
            shader.setUniformValue(
                attenuation.data(), static_cast<float>(settings.GetLightAttentuationFactor()));

            const auto ambientCoefficient = "allLights[" + indexAsString + "].ambientCoefficient";
            shader.setUniformValue(
                ambientCoefficient.data(),
                static_cast<float>(settings.GetAmbientLightCoefficient()));
        }
    }

    /**
     * @brief Determines the appropriate color for the file based on the user-configurable color set
     * in the color.json file.
     *
     * @param[in] node               The node whose color needs to be restored.
     * @param[in] settings           The settings that will determine the color.
     *
     * @returns The appropriate color found in the color map.
     */
    boost::optional<QVector3D>
    DetermineColorFromExtension(const Tree<VizBlock>::Node& node, const Settings::Manager& settings)
    {
        const auto& colorMap = settings.GetFileColorMap();
        const auto categoryItr = colorMap.find(settings.GetActiveColorScheme());
        if (categoryItr == std::end(colorMap)) {
            return boost::none;
        }

        const auto extensionItr = categoryItr->second.find(node->file.extension);
        if (extensionItr == std::end(categoryItr->second)) {
            return boost::none;
        }

        return extensionItr->second;
    }

    /**
     * @brief Restores the previously selected node to its non-selected color based on the rendering
     * settings.
     *
     * @param[in] node               The node whose color needs to be restored.
     * @param[in] settings           The settings that will determine the color.
     *
     * @returns The color to restore the node to.
     */
    QVector3D RestoreColor(const Tree<VizBlock>::Node& node, const Settings::Manager& settings)
    {
        const auto fileColor = DetermineColorFromExtension(node, settings);
        if (fileColor) {
            return *fileColor;
        }

        if (node.GetData().file.type != FileType::DIRECTORY) {
            return Constants::Colors::FILE_GREEN;
        }

        if (!settings.GetVisualizationParameters().useDirectoryGradient) {
            return Constants::Colors::WHITE;
        }

        auto* rootNode = &node;
        while (rootNode->GetParent()) {
            rootNode = rootNode->GetParent();
        }

        const auto ratio =
            static_cast<float>(node->file.size) / static_cast<float>((*rootNode)->file.size);

        ColorGradient gradient;
        return gradient.GetColorAtValue(ratio);
    }

    /**
     * @returns The view matrix for the shadow casting light source.
     */
    QMatrix4x4 ComputeLightViewMatrix()
    {
        constexpr auto lightPosition = QVector3D{ -200.f, 500.f, -200.f };
        constexpr auto lightTarget = QVector3D{ 500.f, 0.f, -500.f };
        constexpr auto upVector = QVector3D{ 0.0f, 1.0f, 0.0f };

        QMatrix4x4 view;
        view.lookAt(lightPosition, lightTarget, upVector);

        return view;
    }

    /**
     * @brief Rounds the input value to the nearest multiple.
     *
     * @param value                  The raw input value.
     * @param multiple               World units per texel unit.
     *
     * @returns The rounded value.
     */
    float SnapToNearestTexel(float value, float multiple)
    {
        return std::round(value / multiple) * multiple;
    }

    /**
     * @returns The length of the diagonal of the provided bounding box.
     */
    auto ComputeDiagonal(const BoundingBox& box)
    {
        const auto cornerA = QVector3D{ box.left, box.top, box.near };
        const auto cornerB = QVector3D{ box.right, box.bottom, box.far };

        return static_cast<double>(cornerA.distanceToPoint(cornerB));
    }
} // namespace

namespace Asset
{
    Treemap::Treemap(const Settings::Manager& settings, QOpenGLExtraFunctions& openGL)
        : Base{ settings, openGL }
    {
        m_shouldRender = DetermineVisibilityFromPreferences(AssetName);

        const auto& preferences = m_settingsManager.GetPreferenceMap();
        m_cascadeCount = preferences.GetValueOrDefault(L"shadowMapCascadeCount", 4);
        m_shadowMapResolution = preferences.GetValueOrDefault(L"shadowMapQuality", 4) * 1024;

        const auto& log = spdlog::get(Constants::Logging::DEFAULT_LOG);
        log->info("Shadow map width & height is set at {} pixels.", m_shadowMapResolution);

        m_shadowMaps.reserve(static_cast<std::size_t>(m_cascadeCount));

        for (auto index{ 0u }; index < static_cast<std::size_t>(m_cascadeCount); ++index) {
            auto frameBuffer = std::make_unique<QOpenGLFramebufferObject>(
                m_shadowMapResolution, m_shadowMapResolution,
                QOpenGLFramebufferObject::Attachment::Depth, GL_TEXTURE_2D, GL_R32F);

            m_shadowMaps.emplace_back(std::move(frameBuffer));
        }
    }

    bool Treemap::LoadShaders()
    {
        m_shadowMapShader.addShaderFromSourceFile(
            QOpenGLShader::Vertex, ":/Shaders/shadowMapping.vert");

        m_shadowMapShader.addShaderFromSourceFile(
            QOpenGLShader::Fragment, ":/Shaders/shadowMapping.frag");

        auto success = m_shadowMapShader.link();

        success &= Base::LoadShaders("visualizationVertexShader", "visualizationFragmentShader");
        success &= LoadTexturePreviewShaders();

        return success;
    }

    void Treemap::Initialize()
    {
        InitializeReferenceBlock();
        InitializeBlockTransformations();
        InitializeColors();
        InitializeShadowMachinery();
        InitializeTexturePreviewer();
    }

    void Treemap::InitializeReferenceBlock()
    {
        if (!m_VAO.isCreated()) {
            m_VAO.create();
        }

        const auto referenceBlock = Block{ PrecisePoint{ 0.0, 0.0, 0.0 },
                                           /* width =  */ 1.0,
                                           /* height = */ 1.0,
                                           /* depth =  */ 1.0,
                                           /* generateVertices = */ true };

        m_referenceBlockVertices.clear();
        m_referenceBlockVertices = referenceBlock.GetVerticesAndNormals();

        m_VAO.bind();

        m_referenceBlockBuffer.create();
        m_referenceBlockBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
        m_referenceBlockBuffer.bind();
        m_referenceBlockBuffer.allocate(
            /* data = */ m_referenceBlockVertices.constData(),
            /* count = */ m_referenceBlockVertices.size() * static_cast<int>(sizeof(QVector3D)));

        m_referenceBlockBuffer.bind();

        m_mainShader.enableAttributeArray("vertex");
        m_mainShader.setAttributeBuffer(
            /* name = */ "vertex",
            /* type = */ GL_FLOAT,
            /* offset = */ 0,
            /* tupleSize = */ 3,
            /* stride = */ 2 * sizeof(QVector3D));

        m_mainShader.enableAttributeArray("normal");
        m_mainShader.setAttributeBuffer(
            /* name = */ "normal",
            /* type = */ GL_FLOAT,
            /* offset = */ sizeof(QVector3D),
            /* tupleSize = */ 3,
            /* stride = */ 2 * sizeof(QVector3D));

        m_referenceBlockBuffer.release();
        m_VAO.release();
    }

    void Treemap::InitializeColors()
    {
        if (!m_VAO.isCreated()) {
            m_VAO.create();
        }

        m_VAO.bind();

        m_blockColorBuffer.create();
        m_blockColorBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
        m_blockColorBuffer.bind();
        m_blockColorBuffer.allocate(
            /* data = */ m_blockColors.constData(),
            /* count = */ m_blockColors.size() * 3 * static_cast<int>(sizeof(GLfloat)));

        m_openGL.glEnableVertexAttribArray(0);
        m_openGL.glVertexAttribDivisor(0, 1);
        m_openGL.glVertexAttribPointer(
            /* indx = */ 0,
            /* size = */ 3,
            /* type = */ GL_FLOAT,
            /* normalized = */ GL_FALSE,
            /* stride = */ sizeof(QVector3D),
            /* ptr = */ static_cast<GLvoid*>(nullptr));

        m_blockColorBuffer.release();
        m_VAO.release();
    }

    void Treemap::InitializeBlockTransformations()
    {
        if (!m_VAO.isCreated()) {
            m_VAO.create();
        }

        m_VAO.bind();

        constexpr auto sizeOfVector = static_cast<int>(sizeof(QVector4D));
        constexpr auto sizeOfMatrix = static_cast<int>(sizeof(QMatrix4x4));

        m_blockTransformationBuffer.create();
        m_blockTransformationBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
        m_blockTransformationBuffer.bind();
        m_blockTransformationBuffer.allocate(
            /* data = */ m_blockTransformations.constData(),
            /* count = */ m_blockTransformations.size() * sizeOfMatrix);

        // Row 1 of the matrix:
        m_openGL.glEnableVertexAttribArray(1);
        m_openGL.glVertexAttribDivisor(1, 1);
        m_openGL.glVertexAttribPointer(
            /* indx = */ 1,
            /* size = */ 4,
            /* type = */ GL_FLOAT,
            /* normalized = */ GL_FALSE,
            /* stride = */ sizeOfMatrix,
            /* ptr = */ nullptr);

        // Row 2 of the matrix:
        m_openGL.glEnableVertexAttribArray(2);
        m_openGL.glVertexAttribDivisor(2, 1);
        m_openGL.glVertexAttribPointer(
            /* indx = */ 2,
            /* size = */ 4,
            /* type = */ GL_FLOAT,
            /* normalized = */ GL_FALSE,
            /* stride = */ sizeOfMatrix,
            /* ptr = */ reinterpret_cast<GLvoid*>(1 * sizeOfVector)); // NOLINT

        // Row 3 of the matrix:
        m_openGL.glEnableVertexAttribArray(3);
        m_openGL.glVertexAttribDivisor(3, 1);
        m_openGL.glVertexAttribPointer(
            /* indx = */ 3,
            /* size = */ 4,
            /* type = */ GL_FLOAT,
            /* normalized = */ GL_FALSE,
            /* stride = */ sizeOfMatrix,
            /* ptr = */ reinterpret_cast<GLvoid*>(2 * sizeOfVector)); // NOLINT

        // Row 4 of the matrix:
        m_openGL.glEnableVertexAttribArray(4);
        m_openGL.glVertexAttribDivisor(4, 1);
        m_openGL.glVertexAttribPointer(
            /* indx = */ 4,
            /* size = */ 4,
            /* type = */ GL_FLOAT,
            /* normalized = */ GL_FALSE,
            /* stride = */ sizeOfMatrix,
            /* ptr = */ reinterpret_cast<GLvoid*>(3 * sizeOfVector)); // NOLINT

        m_blockTransformationBuffer.release();
        m_VAO.release();
    }

    void Treemap::InitializeShadowMachinery()
    {
        InitializeShadowMachineryOnMainShader();
        InitializeShadowMachineryOnShadowShader();
    }

    void Treemap::InitializeShadowMachineryOnMainShader()
    {
        m_mainShader.bind();

        const auto shaderID = m_mainShader.programId();

        const auto cascadeBounds = FrustumUtilities::GetCascadeDistances();
        for (auto index{ 0u }; index < cascadeBounds.size(); ++index) {
            const auto indexAsString = std::to_string(index);

            auto variableName = "cascadeBounds[" + indexAsString + "]";
            m_mainShader.setUniformValue(variableName.data(), cascadeBounds[index].second);

            variableName = "shadowMaps[" + indexAsString + "]";
            const auto location = m_openGL.glGetUniformLocation(shaderID, variableName.data());
            m_openGL.glUniform1i(location, static_cast<int>(index));
        }

        m_mainShader.release();
    }

    void Treemap::InitializeShadowMachineryOnShadowShader()
    {
        m_VAO.bind();
        m_referenceBlockBuffer.bind();
        m_shadowMapShader.bind();

        m_shadowMapShader.enableAttributeArray("vertex");
        m_shadowMapShader.setAttributeBuffer(
            /* name = */ "vertex",
            /* type = */ GL_FLOAT,
            /* offset = */ 0,
            /* tupleSize = */ 3,
            /* stride = */ 2 * sizeof(QVector3D));

        m_shadowMapShader.enableAttributeArray("normal");
        m_shadowMapShader.setAttributeBuffer(
            /* name = */ "normal",
            /* type = */ GL_FLOAT,
            /* offset = */ sizeof(QVector3D),
            /* tupleSize = */ 3,
            /* stride = */ 2 * sizeof(QVector3D));

        m_shadowMapShader.release();
        m_referenceBlockBuffer.release();
        m_VAO.release();
    }

    std::uint32_t Treemap::LoadBufferData(const Tree<VizBlock>& tree)
    {
        m_blockTransformations.clear();
        m_blockColors.clear();

        m_blockCount = 0;

        const auto& parameters = m_settingsManager.GetVisualizationParameters();

        for (auto& node : tree) {
            const auto fileIsTooSmall = (node->file.size < parameters.minimumFileSize);
            const auto notTheRightFileType =
                parameters.onlyShowDirectories && node->file.type != FileType::DIRECTORY;

            if (notTheRightFileType || fileIsTooSmall) {
                node->offsetIntoVBO = VizBlock::INVALID_OFFSET;
                continue;
            }

            node->offsetIntoVBO = m_blockCount++;

            const auto& block = node->block;
            const auto& blockOrigin = block.GetOrigin();

            QMatrix4x4 instanceMatrix;
            instanceMatrix.translate(
                static_cast<float>(blockOrigin.x()), static_cast<float>(blockOrigin.y()),
                static_cast<float>(blockOrigin.z()));

            instanceMatrix.scale(
                static_cast<float>(block.GetWidth()), static_cast<float>(block.GetHeight()),
                static_cast<float>(block.GetDepth()));

            m_blockTransformations << instanceMatrix;

            ComputeAppropriateBlockColor(node);
        }

        FindLargestDirectory(tree);

        Expects(m_blockColors.size() == m_blockTransformations.size());
        Expects(m_blockColors.size() == static_cast<int>(m_blockCount));

        return m_blockCount;
    }

    void Treemap::ReloadColorBufferData(const Tree<VizBlock>& tree)
    {
        m_blockColors.clear();

        const auto& parameters = m_settingsManager.GetVisualizationParameters();

        for (const auto& node : tree) {
            const auto fileIsTooSmall = (node->file.size < parameters.minimumFileSize);
            const auto notTheRightFileType =
                parameters.onlyShowDirectories && node->file.type != FileType::DIRECTORY;

            if (notTheRightFileType || fileIsTooSmall) {
                continue;
            }

            ComputeAppropriateBlockColor(node);
        }
    }

    void Treemap::FindLargestDirectory(const Tree<VizBlock>& tree)
    {
        std::uintmax_t largestDirectory = std::numeric_limits<std::uintmax_t>::min();

        for (auto& node : tree) {
            if (node.GetData().file.type != FileType::DIRECTORY) {
                continue;
            }

            const auto directorySize = node.GetData().file.size;
            if (directorySize > largestDirectory) {
                largestDirectory = directorySize;
            }
        }

        Expects(largestDirectory > std::numeric_limits<std::uintmax_t>::min());

        m_largestDirectorySize = largestDirectory;
    }

    QVector3D Treemap::ComputeGradientColor(const Tree<VizBlock>::Node& node)
    {
        const auto blockSize = static_cast<long double>(node.GetData().file.size);
        const auto ratio = blockSize / static_cast<long double>(m_largestDirectorySize);

        const auto finalColor = m_directoryColorGradient.GetColorAtValue(static_cast<float>(ratio));
        return finalColor;
    }

    void Treemap::ComputeAppropriateBlockColor(const Tree<VizBlock>::Node& node)
    {
        // @todo Need to also take into consideration whether the node is highlighted or selected,
        // since we don't want to get out of sync with the controller's view of the world.

        if (m_settingsManager.GetActiveColorScheme() != Constants::ColorScheme::DEFAULT) {
            const auto fileColor = DetermineColorFromExtension(node, m_settingsManager);
            if (fileColor) {
                m_blockColors << *fileColor;
                return;
            }
        }

        if (node->file.type == FileType::DIRECTORY) {
            if (m_settingsManager.GetVisualizationParameters().useDirectoryGradient) {
                m_blockColors << ComputeGradientColor(node);
            } else {
                m_blockColors << Constants::Colors::WHITE;
            }
        } else if (node->file.type == FileType::REGULAR) {
            m_blockColors << Constants::Colors::FILE_GREEN;
        }
    }

    std::uint32_t Treemap::GetBlockCount() const
    {
        return m_blockCount;
    }

    bool Treemap::IsAssetLoaded() const
    {
        return !(m_blockTransformations.empty() && m_blockColors.empty());
    }

    float Treemap::ComputeWorldUnitsPerTexel(const BoundingBox& boundingBox)
    {
        // In order to reduce shadow shimmering, we'll attempt to snap the orthogonal projection
        // matrix for the shadow cascades to the nearest texel. This appears to significantly reduce
        // the shimmering effect, while not fully eliminating it entirely.

        const auto diagonal = ComputeDiagonal(boundingBox);
        m_maxBoundingBoxDiagonal = std::max(diagonal, m_maxBoundingBoxDiagonal);
        const auto worldUnitsPerTexel = m_maxBoundingBoxDiagonal / m_shadowMapResolution;

        return static_cast<float>(worldUnitsPerTexel);
    }

    void Treemap::ComputeShadowMapProjectionViewMatrices(const Camera& camera)
    {
        const auto view = ComputeLightViewMatrix();
        const auto boundingBoxes = ComputeFrustumSplitBoundingBoxes(camera, view, m_cascadeCount);

        constexpr auto nearPlane{ 200 };
        constexpr auto farPlane{ 1500 };

        for (auto index{ 0u }; index < static_cast<std::size_t>(m_cascadeCount); ++index) {
            const auto& boundingBox = boundingBoxes[index];
            const auto worldUnitsPerTexel = ComputeWorldUnitsPerTexel(boundingBox);

            QMatrix4x4 projection;
            projection.ortho(
                SnapToNearestTexel(boundingBox.left, worldUnitsPerTexel),
                SnapToNearestTexel(boundingBox.right, worldUnitsPerTexel),
                SnapToNearestTexel(boundingBox.bottom, worldUnitsPerTexel),
                SnapToNearestTexel(boundingBox.top, worldUnitsPerTexel), nearPlane, farPlane);

            const QMatrix4x4 model;
            auto projectionViewMatrix = projection * view * model;
            m_shadowMaps[index].projectionViewMatrix = projectionViewMatrix;
        }
    }

    void Treemap::RenderShadowPass(const Camera& camera)
    {
        ComputeShadowMapProjectionViewMatrices(camera);

        m_openGL.glViewport(
            /* x = */ 0,
            /* y = */ 0,
            /* width = */ static_cast<std::int32_t>(m_shadowMapResolution),
            /* height = */ static_cast<std::int32_t>(m_shadowMapResolution));

        m_shadowMapShader.bind();
        m_VAO.bind();

        for (auto index{ 0u }; index < static_cast<std::size_t>(m_cascadeCount); ++index) {
            m_shadowMaps[index].framebuffer->bind();

            m_shadowMapShader.setUniformValue(
                "lightProjectionViewMatrix", m_shadowMaps[index].projectionViewMatrix);

            m_openGL.glClear(GL_DEPTH_BUFFER_BIT);

            static constexpr float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            m_openGL.glClearBufferfv(GL_COLOR, 0, white);

            m_openGL.glDrawArraysInstanced(
                /* mode = */ GL_TRIANGLES,
                /* first = */ 0,
                /* count = */ m_referenceBlockVertices.size(),
                /* instanceCount = */ m_blockColors.size());

            m_shadowMaps[index].framebuffer->release();
        }

        m_VAO.release();
        m_shadowMapShader.release();

        const auto& viewport = camera.GetViewport();
        m_openGL.glViewport(0, 0, viewport.width(), viewport.height());
    }

    void Treemap::RenderMainPass(const Camera& camera, const std::vector<Light>& lights)
    {
        m_mainShader.bind();

        const auto shouldShowShadows = m_settingsManager.ShouldRenderShadows();
        const auto shouldShowCascadeSplits = m_settingsManager.ShouldShowCascadeSplits();

        m_mainShader.setUniformValue(
            "cameraProjectionViewMatrix", camera.GetProjectionViewMatrix());
        m_mainShader.setUniformValue("cameraPosition", camera.GetPosition());

        // @todo The following variables don't need to be set with every pass...
        m_mainShader.setUniformValue(
            "materialShininess", static_cast<float>(m_settingsManager.GetMaterialShininess()));
        m_mainShader.setUniformValue("shouldShowCascadeSplits", shouldShowCascadeSplits);
        m_mainShader.setUniformValue("shouldShowShadows", shouldShowShadows);

        SetUniformLights(lights, m_settingsManager, m_mainShader);

        if (shouldShowShadows) {
            Expects(m_shadowMaps.size() == static_cast<std::size_t>(m_cascadeCount));

            for (auto index{ 0u }; index < static_cast<std::size_t>(m_cascadeCount); ++index) {
                const auto matrix = "lightProjectionViewMatrices[" + std::to_string(index) + "]";
                m_mainShader.setUniformValue(
                    matrix.data(), m_shadowMaps[index].projectionViewMatrix);

                m_openGL.glActiveTexture(GL_TEXTURE0 + index);
                m_openGL.glBindTexture(GL_TEXTURE_2D, m_shadowMaps[index].framebuffer->texture());
            }
        }

        m_VAO.bind();

        m_openGL.glDrawArraysInstanced(
            /* mode = */ GL_TRIANGLES,
            /* first = */ 0,
            /* count = */ m_referenceBlockVertices.size(),
            /* instanceCount = */ m_blockColors.size());

        m_VAO.release();

        m_mainShader.release();
    }

    void Treemap::Render(const Camera& camera, const std::vector<Light>& lights)
    {
        if (!IsAssetLoaded()) {
            return;
        }

        if (m_settingsManager.ShouldRenderShadows()) {
            RenderShadowPass(camera);
        }

        RenderMainPass(camera, lights);

        // RenderDepthMapPreview(1); //< @note Enable this to render the shadow map to the screen.
    }

    void Treemap::Refresh()
    {
        InitializeReferenceBlock();
        InitializeColors();
        InitializeBlockTransformations();
    }

    void Treemap::UpdateVBO(const Tree<VizBlock>::Node& node, Asset::Event action)
    {
        Expects(m_VAO.isCreated());
        Expects(m_blockColorBuffer.isCreated());

        if (node->offsetIntoVBO > m_blockCount) {
            return;
        }

        constexpr auto colorTupleSize = static_cast<int>(sizeof(QVector3D));
        const auto offsetIntoColorBuffer = node->offsetIntoVBO * colorTupleSize;

        QVector3D newColor;

        switch (action) {
            case Asset::Event::SELECTED: {
                newColor = Constants::Colors::CANARY_YELLOW;
                break;
            }
            case Asset::Event::HIGHLIGHTED: {
                newColor = Constants::Colors::SLATE_GRAY;
                break;
            }
            case Asset::Event::UNSELECTED: {
                // @todo Update restoration logic to account for colors that represent file system
                // modifications.

                newColor = RestoreColor(node, m_settingsManager);
                break;
            }
            case Asset::Event::TOUCHED: {
                newColor = Constants::Colors::BABY_BLUE; //< @todo Pick better color.
                break;
            }
            case Asset::Event::RENAMED: {
                newColor = Constants::Colors::HOT_PINK; //< @todo Pick better color.
                break;
            }
            case Asset::Event::DELETED: {
                newColor = Constants::Colors::CORAL; //< @todo Pick better color.
                break;
            }
        }

        m_VAO.bind();
        m_blockColorBuffer.bind();

        m_openGL.glBufferSubData(
            /* target = */ GL_ARRAY_BUFFER,
            /* offset = */ offsetIntoColorBuffer,
            /* size = */ colorTupleSize,
            /* data = */ &newColor);

        m_blockColorBuffer.release();
        m_VAO.release();
    }

    bool Treemap::LoadTexturePreviewShaders()
    {
        if (!m_texturePreviewShader.addShaderFromSourceFile(
                QOpenGLShader::Vertex, ":/Shaders/texturePreview.vert")) {
            std::cout << "Error loading vertex shader!" << std::endl;
        }

        if (!m_texturePreviewShader.addShaderFromSourceFile(
                QOpenGLShader::Fragment, ":/Shaders/texturePreview.frag")) {
            std::cout << "Error loading fragment shader!" << std::endl;
        }

        return m_texturePreviewShader.link();
    }

    bool Treemap::InitializeTexturePreviewer()
    {
        static constexpr int coordinates[4][3] = {
            { +1, -1, -1 }, { -1, -1, -1 }, { -1, +1, -1 }, { +1, +1, -1 }
        };

        m_texturePreviewShader.bindAttributeLocation("vertex", TEXTURE_PREVIEWER_VERTEX_ATTRIBUTE);

        m_texturePreviewShader.bindAttributeLocation(
            "texCoord", TEXTURE_PREVIEWER_TEXCOORD_ATTRIBUTE);

        QVector<GLfloat> vertexData;
        for (int i = 0; i < 4; ++i) {
            // Vertex position:
            vertexData.append(coordinates[i][0]);
            vertexData.append(coordinates[i][1]);
            vertexData.append(coordinates[i][2]);

            // Texture coordinate:
            vertexData.append(i == 0 || i == 3);
            vertexData.append(i == 0 || i == 1);
        }

        m_texturePreviewVertexBuffer.create();
        m_texturePreviewVertexBuffer.bind();

        m_texturePreviewVertexBuffer.allocate(
            vertexData.constData(), vertexData.count() * static_cast<int>(sizeof(GLfloat)));

        m_texturePreviewVertexBuffer.release();

        return true;
    }

    void Treemap::RenderDepthMapPreview(std::size_t index)
    {
        // Simply using Normalized Device Coordinates (NDC), and an arbitrary choice of view planes.
        QMatrix4x4 viewMatrix;
        viewMatrix.ortho(-1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1000.0f);
        viewMatrix.translate(0.0f, 0.0f, -1.0f);

        m_texturePreviewVertexBuffer.bind();

        m_texturePreviewShader.bind();
        m_texturePreviewShader.setUniformValue("matrix", viewMatrix);
        m_texturePreviewShader.enableAttributeArray(TEXTURE_PREVIEWER_VERTEX_ATTRIBUTE);
        m_texturePreviewShader.enableAttributeArray(TEXTURE_PREVIEWER_TEXCOORD_ATTRIBUTE);

        m_texturePreviewShader.setAttributeBuffer(
            TEXTURE_PREVIEWER_VERTEX_ATTRIBUTE, GL_FLOAT,
            /* offset = */ 0,
            /* tupleSize = */ 3,
            /* stride = */ 5 * sizeof(GLfloat));

        m_texturePreviewShader.setAttributeBuffer(
            TEXTURE_PREVIEWER_TEXCOORD_ATTRIBUTE, GL_FLOAT,
            /* offset = */ 3 * sizeof(GLfloat),
            /* tupleSize = */ 2,
            /* stride = */ 5 * sizeof(GLfloat));

        m_openGL.glActiveTexture(GL_TEXTURE0);
        m_openGL.glBindTexture(GL_TEXTURE_2D, m_shadowMaps[index].framebuffer->texture());

        m_openGL.glDrawArrays(
            /* mode = */ GL_TRIANGLE_FAN,
            /* first = */ 0,
            /* count = */ 4);

        m_texturePreviewShader.release();
        m_texturePreviewVertexBuffer.release();
    }
} // namespace Asset
