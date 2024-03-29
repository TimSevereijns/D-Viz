#include "View/Scene/Assets/frustumAsset.h"
#include "Utilities/viewFrustum.h"
#include "constants.h"

#include <algorithm>

#include <cmath>

namespace
{
    /**
     * @brief ComputeLightTransformationMatrix
     *
     * @param camera
     *
     * @returns
     */
    QMatrix4x4 ComputeLightViewMatrix()
    {
        constexpr auto lightPosition = QVector3D{ -200.f, 500.f, -200.f };
        constexpr auto lightTarget = QVector3D{ 500.f, 0.f, -500.f };

        QMatrix4x4 projection;
        projection.ortho(-600, 600, -600, 600, 10, 1500);

        QMatrix4x4 model;
        QMatrix4x4 view;
        view.lookAt(lightPosition, lightTarget, QVector3D{ 0.0f, 1.0f, 0.0f });
        return projection * view * model;
    }

    /**
     * @brief Calculates and sets the vertices needed to visualize the Axis Aligned Bounding Boxes
     * for each of the frustum splits.
     *
     * @param[in, out] frustumAsset  The main frustum scene asset.
     * @param[in] renderCamera       The main camera used to render the scene. Mainly use is to get
     *                               the aspect ratio of the outline correct.
     * @param[in] shadowCamera       The camera that is to render the scene from the light caster's
     *                               perspective.
     */
    void GenerateCascadeBoundingBoxes(
        Assets::Frustum& frustumAsset, const Camera& renderCamera, const QMatrix4x4& worldToLight)
    {
        const auto cascadeDistances = FrustumUtilities::GetCascadeDistances();
        const auto cascadeCount{ cascadeDistances.size() };

        std::vector<std::vector<QVector3D>> frusta;
        frusta.reserve(cascadeCount);

        auto mutableCamera = renderCamera;
        for (const auto& nearAndFarPlanes : cascadeDistances) {
            mutableCamera.SetNearPlane(nearAndFarPlanes.first);
            mutableCamera.SetFarPlane(nearAndFarPlanes.second);

            frusta.emplace_back(FrustumUtilities::ComputeFrustumCorners(mutableCamera));
        }

        QVector<QVector3D> vertices;
        vertices.reserve(24 * static_cast<int>(cascadeCount));

        for (auto& frustum : frusta) {
            auto minX = std::numeric_limits<float>::max();
            auto maxX = std::numeric_limits<float>::lowest();

            auto minY = std::numeric_limits<float>::max();
            auto maxY = std::numeric_limits<float>::lowest();

            auto minZ = std::numeric_limits<float>::max();
            auto maxZ = std::numeric_limits<float>::lowest();

            // Compute bounding box in light space:
            for (auto& vertex : frustum) {
                const auto mappedVertex = worldToLight.map(vertex);

                minX = std::min(minX, mappedVertex.x());
                maxX = std::max(maxX, mappedVertex.x());

                minY = std::min(minY, mappedVertex.y());
                maxY = std::max(maxY, mappedVertex.y());

                minZ = std::min(minZ, mappedVertex.z());
                maxZ = std::max(maxZ, mappedVertex.z());
            }

            // clang-format off
            vertices
                // Near plane:
                << QVector3D{ minX, maxY, minZ } << QVector3D{ maxX, maxY, minZ }
                << QVector3D{ maxX, maxY, minZ } << QVector3D{ maxX, minY, minZ }
                << QVector3D{ maxX, minY, minZ } << QVector3D{ minX, minY, minZ }
                << QVector3D{ minX, minY, minZ } << QVector3D{ minX, maxY, minZ }
                // Far plane:
                << QVector3D{ minX, maxY, maxZ } << QVector3D{ maxX, maxY, maxZ }
                << QVector3D{ maxX, maxY, maxZ } << QVector3D{ maxX, minY, maxZ }
                << QVector3D{ maxX, minY, maxZ } << QVector3D{ minX, minY, maxZ }
                << QVector3D{ minX, minY, maxZ } << QVector3D{ minX, maxY, maxZ }
                // Connect the planes:
                << QVector3D{ minX, maxY, minZ } << QVector3D{ minX, maxY, maxZ }
                << QVector3D{ maxX, maxY, minZ } << QVector3D{ maxX, maxY, maxZ }
                << QVector3D{ maxX, minY, minZ } << QVector3D{ maxX, minY, maxZ }
                << QVector3D{ minX, minY, minZ } << QVector3D{ minX, minY, maxZ };
            // clang-format on
        }

        const auto lightToWorld = worldToLight.inverted();

        // Transform points back to camera space for visualization:
        for (auto index{ 0 }; index < vertices.size(); ++index) {
            vertices[index] = lightToWorld.map(vertices[index]);
        }

        QVector<QVector3D> colors;
        colors.reserve(static_cast<int>(24 * cascadeCount));

        for (auto index{ 0 }; index < vertices.size(); ++index) {
            colors << Constants::Colors::Green;
        }

        frustumAsset.AddVertexCoordinates(std::move(vertices));
        frustumAsset.AddVertexColors(std::move(colors));
    }

    /**
     * @brief Helper function to draw the frustum belonging to single stationary scene camera.
     *
     * @param[in, out] frustumAsset  The main frustum scene asset.
     * @param[in] camera             The main camera used to render the scene. Mainly use is to get
     *                               the aspect ratio of the outline correct.
     */
    void GenerateCameraFrusta(Assets::Frustum& frustumAsset, const Camera& camera)
    {
        Camera mutableCamera = camera;

        constexpr auto cascadeCount{ 3 };
        const auto cascades =
            FrustumUtilities::ComputeCascadeDistances(cascadeCount, mutableCamera);

        std::vector<std::vector<QVector3D>> frusta;
        frusta.reserve(cascadeCount);

        for (const auto& nearAndFarPlanes : cascades) {
            mutableCamera.SetNearPlane(nearAndFarPlanes.first);
            mutableCamera.SetFarPlane(nearAndFarPlanes.second);

            frusta.emplace_back(FrustumUtilities::GenerateFrustumPoints(mutableCamera));
        }

        for (auto& frustum : frusta) {
            auto vertices = QVector<QVector3D>(std::begin(frustum), std::end(frustum));

            QVector<QVector3D> colors;
            colors.reserve(vertices.size());

            for (auto index{ 0 }; index < vertices.size(); ++index) {
                colors << Constants::Colors::HotPink;
            }

            frustumAsset.AddVertexCoordinates(std::move(vertices));
            frustumAsset.AddVertexColors(std::move(colors));
        }
    }
} // namespace

namespace Assets
{
    Frustum::Frustum(const Controller& controller, QOpenGLExtraFunctions& openGL)
        : Line{ controller, openGL }
    {
        m_shouldRender = m_persistentSettings.ShouldRenderFrusta();
    }

    void Frustum::Render(const Camera& camera, const std::vector<Light>& /*lights*/)
    {
        if (!m_shouldRender) {
            return;
        }

        m_mainShader.bind();
        m_mainShader.setUniformValue("mvpMatrix", camera.GetProjectionViewMatrix());

        m_VAO.bind();

        m_openGL.glLineWidth(2);

        m_openGL.glDrawArrays(
            /* mode = */ GL_LINES,
            /* first = */ 0,
            /* count = */ m_rawVertices.size());

        m_openGL.glLineWidth(1);

        m_mainShader.release();
        m_VAO.release();
    }

    void Frustum::GenerateFrusta(const Camera& camera)
    {
        ClearBuffers();

        GenerateCameraFrusta(*this, camera);
        GenerateCascadeBoundingBoxes(*this, camera, ComputeLightViewMatrix());

        Refresh();
    }
} // namespace Assets
