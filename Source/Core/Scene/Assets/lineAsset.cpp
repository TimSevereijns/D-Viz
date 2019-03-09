#include "Scene/Assets/lineAsset.h"

namespace Asset
{
    Line::Line(const Settings::Manager& settings, QOpenGLExtraFunctions& openGL)
        : Base{ settings, openGL }
    {
    }

    bool Line::LoadShaders()
    {
        return Base::LoadShaders("simpleLineVertexShader", "simpleLineFragmentShader");
    }

    void Line::Initialize()
    {
        InitializeVertexBuffers();
        InitializeColorBuffers();
    }

    void Line::InitializeVertexBuffers()
    {
        if (!m_VAO.isCreated()) {
            m_VAO.create();
        }

        m_VAO.bind();

        m_vertexBuffer.create();
        m_vertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
        m_vertexBuffer.bind();
        m_vertexBuffer.allocate(
            /* data = */ m_rawVertices.constData(),
            /* count = */ m_rawVertices.size() * 3 * static_cast<int>(sizeof(GLfloat)));

        m_mainShader.bind();
        m_vertexBuffer.bind();

        m_mainShader.enableAttributeArray("vertex");
        m_mainShader.setAttributeBuffer(
            /* name = */ "vertex",
            /* type = */ GL_FLOAT,
            /* offset = */ 0,
            /* tupleSize = */ 3);

        m_vertexBuffer.release();
        m_mainShader.release();
        m_VAO.release();
    }

    void Line::InitializeColorBuffers()
    {
        if (!m_VAO.isCreated()) {
            m_VAO.create();
        }

        m_VAO.bind();

        m_colorBuffer.create();
        m_colorBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
        m_colorBuffer.bind();
        m_colorBuffer.allocate(
            /* data = */ m_rawColors.constData(),
            /* count = */ m_rawColors.size() * 3 * static_cast<int>(sizeof(GLfloat)));

        m_colorBuffer.bind();
        m_mainShader.enableAttributeArray("color");
        m_mainShader.setAttributeBuffer(
            /* name = */ "color",
            /* type = */ GL_FLOAT,
            /* offset = */ 0,
            /* tupleSize = */ 3);

        m_colorBuffer.release();
        m_VAO.release();
    }

    void Line::Render(const Camera& camera, const std::vector<Light>&)
    {
        if (!m_shouldRender) {
            return;
        }

        m_mainShader.bind();
        m_mainShader.setUniformValue("mvpMatrix", camera.GetProjectionViewMatrix());

        m_VAO.bind();

        m_openGL.glDrawArrays(
            /* mode = */ GL_LINES,
            /* first = */ 0,
            /* count = */ m_rawVertices.size());

        m_mainShader.release();
        m_VAO.release();
    }

    void Line::Refresh()
    {
        Initialize();
    }
} // namespace Asset
