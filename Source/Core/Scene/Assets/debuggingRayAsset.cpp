#include "Scene/Assets/debuggingRayAsset.h"

namespace Asset
{
    DebuggingRay::DebuggingRay(const Settings::Manager& settings, QOpenGLExtraFunctions& openGL)
        : Line{ settings, openGL }
    {
        m_shouldRender = DetermineVisibilityFromPreferences(L"DebuggingRay");
    }

    void DebuggingRay::Render(const Camera& camera, const std::vector<Light>&)
    {
        if (!m_shouldRender) {
            return;
        }

        m_mainShader.bind();
        m_mainShader.setUniformValue("mvpMatrix", camera.GetProjectionViewMatrix());

        m_VAO.bind();

        m_openGL.glLineWidth(3);

        m_openGL.glDrawArrays(
            /* mode = */ GL_LINES,
            /* first = */ 0,
            /* count = */ m_rawVertices.size());

        m_openGL.glLineWidth(1);

        m_mainShader.release();
        m_VAO.release();
    }
} // namespace Asset
