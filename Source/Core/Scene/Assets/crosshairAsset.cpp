#include "Scene/Assets/crosshairAsset.h"

#include <iostream>

namespace
{
    /**
     * @brief Generates the vertices needed to represent the crosshair.
     *
     * @param[in] center             The location at which to center the crosshair.
     *
     * @returns All the vertices needed to draw the crosshair.
     */
    QVector<QVector3D> CreateCrosshairVertices(const QPoint& center)
    {
        QVector<QVector3D> vertices;
        vertices.reserve(4);
        vertices << QVector3D{ static_cast<float>(center.x() - 20), static_cast<float>(center.y()),
                               -4.0f }
                 << QVector3D{ static_cast<float>(center.x() + 20), static_cast<float>(center.y()),
                               -4.0f }
                 << QVector3D{ static_cast<float>(center.x()), static_cast<float>(center.y() - 20),
                               -4.0f }
                 << QVector3D{ static_cast<float>(center.x()), static_cast<float>(center.y() + 20),
                               -4.0f };

        return vertices;
    }

    /**
     * @brief Generates the color data needed to color the crosshair.
     *
     * @returns The color of each vertex needed to represent the crosshair.
     * @see CreateCrosshairVertices
     */
    QVector<QVector3D> CreateCrosshairColors()
    {
        QVector<QVector3D> colors;
        colors.reserve(4);
        colors << QVector3D{ 1.0f, 1.0f, 1.0f } << QVector3D{ 1.0f, 1.0f, 1.0f }
               << QVector3D{ 1.0f, 1.0f, 1.0f } << QVector3D{ 1.0f, 1.0f, 1.0f };

        return colors;
    }
} // namespace

namespace Assets
{
    Crosshair::Crosshair(const Settings::Manager& settings, QOpenGLExtraFunctions& openGL)
        : Line{ settings, openGL }
    {
        m_shouldRender = DetermineVisibilityFromPreferences(AssetName);
        m_rawColors = CreateCrosshairColors();
    }

    void Crosshair::SetCrosshairLocation(const QPoint& canvasCenter)
    {
        m_rawVertices = CreateCrosshairVertices(canvasCenter);

        Refresh();
    }

    void Crosshair::Render(const Camera& camera, const std::vector<Light>&)
    {
        if (!m_shouldRender) {
            return;
        }

        const auto& viewport = camera.GetViewport();
        QMatrix4x4 orthoMatrix;
        orthoMatrix.ortho(
            viewport.left(), viewport.right(), viewport.bottom(), viewport.top(),
            camera.GetNearPlane(), camera.GetFarPlane());

        QMatrix4x4 identityMatrix;
        identityMatrix.setToIdentity();

        const auto mvpMatrix = orthoMatrix * identityMatrix;

        m_mainShader.bind();
        m_mainShader.setUniformValue("mvpMatrix", mvpMatrix);

        m_VAO.bind();

        m_openGL.glLineWidth(2);
        m_openGL.glDrawArrays(
            /* mode = */ GL_LINES,
            /* first = */ 0,
            /* count = */ m_rawVertices.size());

        m_VAO.release();
        m_mainShader.release();
    }
} // namespace Asset
