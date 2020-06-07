#include "View/Scene/Assets/lightMarkerAsset.h"

namespace Assets
{
    LightMarker::LightMarker(const Controller& controller, QOpenGLExtraFunctions& openGL)
        : Line{ controller, openGL }
    {
        m_shouldRender = m_persistentSettings.ShouldRenderLightMarkers();
    }

    void LightMarker::Render(const Camera& camera, const std::vector<Light>&)
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
} // namespace Assets
