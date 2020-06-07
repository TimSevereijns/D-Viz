#include "View/Scene/Assets/originMarkerAsset.h"

#include <gsl/string_span>

namespace
{
    /**
     * @brief Creates the vertices needed to render the coordinate system origin marker.
     *
     * @returns A vector of vertices.
     */
    auto CreateMarkerVertices()
    {
        const float axisLength = Constants::Visualization::RootBlockWidth;

        QVector<QVector3D> vertices;
        vertices.reserve(6);
        vertices << QVector3D{ 0.0f, 0.0f, 0.0f } << QVector3D{ axisLength, 0.0f, 0.0f }   // X-axis
                 << QVector3D{ 0.0f, 0.0f, 0.0f } << QVector3D{ 0.0f, 100.0f, 0.0f }       // Y-axis
                 << QVector3D{ 0.0f, 0.0f, 0.0f } << QVector3D{ 0.0f, 0.0f, -axisLength }; // Z-axis

        return vertices;
    }

    /**
     * @brief Creates the vertex colors needed to paint the origin marker.
     *
     * @returns A vector of vertex colors.
     */
    auto CreateMarkerColors()
    {
        QVector<QVector3D> colors;
        colors.reserve(6);
        colors << QVector3D{ 1.0f, 0.0f, 0.0f } << QVector3D{ 1.0f, 0.0f, 0.0f }  // X-axis (red)
               << QVector3D{ 0.0f, 1.0f, 0.0f } << QVector3D{ 0.0f, 1.0f, 0.0f }  // Y-axis (green)
               << QVector3D{ 0.0f, 0.0f, 1.0f } << QVector3D{ 0.0f, 0.0f, 1.0f }; // Z-axis (blue)

        return colors;
    }
} // namespace

namespace Assets
{
    OriginMarker::OriginMarker(const Controller& controller, QOpenGLExtraFunctions& openGL)
        : Line{ controller, openGL }
    {
        m_shouldRender = m_persistentSettings.ShouldRenderGrid();
        m_rawVertices = CreateMarkerVertices();
        m_rawColors = CreateMarkerColors();
    }

    void OriginMarker::Render(const Camera& camera, const std::vector<Light>&)
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

        m_mainShader.release();
        m_VAO.release();
    }
} // namespace Assets
