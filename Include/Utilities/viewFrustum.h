#ifndef VIEWFRUSTUM_HPP
#define VIEWFRUSTUM_HPP

#include "View/Viewport/camera.h"

#include <cmath>
#include <utility>
#include <vector>

#include <QMatrix4x4>
#include <QVector3D>

namespace FrustumUtilities
{
    /**
     * @brief Compute frustum corners
     *
     * @param worldToView
     */
    inline std::vector<QVector3D> ComputeFrustumCorners(const QMatrix4x4& worldToView)
    {
        std::vector<QVector3D> unitCube = { { -1, -1, -1 }, { +1, -1, -1 }, { +1, +1, -1 },
                                            { -1, +1, -1 }, { -1, -1, +1 }, { +1, -1, +1 },
                                            { +1, +1, +1 }, { -1, +1, +1 } };

        for (auto& corner : unitCube) {
            corner = worldToView.map(corner);
        }

        return unitCube;
    }

    /**
     * @brief Generates all of the frustum vertices for the specified camera.
     *
     * @param[in] camera             Main scene camera.
     */
    inline std::vector<QVector3D> ComputeFrustumCorners(const Camera& camera)
    {
        const auto worldToView = camera.GetProjectionViewMatrix().inverted();
        return ComputeFrustumCorners(worldToView);
    }

    /**
     * @brief Generates a frustum outline.
     *
     * @param[in] camera             Main scene camera.
     * @param[in, out] frustumAsset  The main frustum scene asset.
     */
    template <typename T> inline std::vector<QVector3D> GenerateFrustumPoints(const T& view)
    {
        const auto frustum = ComputeFrustumCorners(view);

        const std::vector<QVector3D> vertices = { // Near plane outline:
                                                  frustum[0], frustum[1], frustum[1], frustum[2],
                                                  frustum[2], frustum[3], frustum[3], frustum[0],
                                                  // Far plane outline:
                                                  frustum[4], frustum[5], frustum[5], frustum[6],
                                                  frustum[6], frustum[7], frustum[7], frustum[4],
                                                  // Side plane outline:
                                                  frustum[0], frustum[4], frustum[1], frustum[5],
                                                  frustum[2], frustum[6], frustum[3], frustum[7]
        };

        return vertices;
    }

    /**
     * @brief Computes the ideal split locations for each frustum cascade.
     *
     * @param[in] cascadeCount       The desired number of shadow mapping cascades.
     * @param[in] camera             The main scene camera.
     */
    inline std::vector<std::pair<float, float>>
    ComputeCascadeDistances(int cascadeCount, float nearPlane, float farPlane)
    {
        const auto planeRatio = farPlane / nearPlane;

        auto previousCascadeStart = nearPlane;

        std::vector<std::pair<float, float>> cascadeDistances;
        for (auto index = 1.0; index < cascadeCount; ++index) {
            const auto cascade =
                nearPlane *
                std::pow(static_cast<float>(planeRatio), static_cast<float>(index / cascadeCount));

            cascadeDistances.emplace_back(previousCascadeStart, cascade);
            previousCascadeStart = cascade;
        }

        cascadeDistances.emplace_back(previousCascadeStart, farPlane);

        return cascadeDistances;
    }

    /**
     * @overload
     */
    inline std::vector<std::pair<float, float>>
    ComputeCascadeDistances(int cascadeCount, const Camera& camera)
    {
        const auto nearPlane = camera.GetNearPlane();
        const auto farPlane = camera.GetFarPlane();

        return ComputeCascadeDistances(cascadeCount, nearPlane, farPlane);
    }

    /**
     * @brief Retrieve hard-coded split locations for each frustum cascade.
     */
    inline std::vector<std::pair<float, float>> GetCascadeDistances()
    {
        static const std::vector<std::pair<float, float>> cascadeDistances = {
            std::make_pair(1.0f, 25.0f), std::make_pair(25.0f, 100.0f),
            std::make_pair(100.0f, 500.0f), std::make_pair(500.0f, 1500.0f)
        };

        return cascadeDistances;
    }
} // namespace FrustumUtilities

#endif // VIEWFRUSTUM_HPP
