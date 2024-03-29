#include "View/Scene/Assets/debuggingRayAsset.h"

namespace Assets
{
    DebuggingRay::DebuggingRay(const Controller& controller, QOpenGLExtraFunctions& openGL)
        : Line{ controller, openGL }
    {
        m_shouldRender = false;
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
} // namespace Assets
