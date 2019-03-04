#include "Scene/lightMarkerAsset.h"

namespace Asset
{
    LightMarker::LightMarker(const Settings::Manager& settings, QOpenGLExtraFunctions& openGL)
        : Line{ settings, openGL }
    {
        m_shouldRender = DetermineVisibilityFromPreferences(AssetName);
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
} // namespace Asset
