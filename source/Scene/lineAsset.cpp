#include "lineAsset.h"

namespace Asset
{
   Line::Line(
      QOpenGLExtraFunctions& openGL,
      bool isInitiallyVisible)
      :
      Base{ openGL, isInitiallyVisible }
   {
   }

   bool Line::LoadShaders()
   {
      return Base::LoadShaders("simpleLineVertexShader", "simpleLineFragmentShader");
   }

   bool Line::Initialize()
   {
      const bool vertexBuffersLoadedSuccessfully = InitializeVertexBuffers();
      const bool colorBuffersLoadedSuccessfully = InitializeColorBuffers();

      const bool overallSuccess = vertexBuffersLoadedSuccessfully && colorBuffersLoadedSuccessfully;
      assert(overallSuccess);

      return overallSuccess;
   }

   bool Line::InitializeVertexBuffers()
   {
      if (!m_VAO.isCreated())
      {
         m_VAO.create();
      }

      m_VAO.bind();

      m_vertexBuffer.create();
      m_vertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
      m_vertexBuffer.bind();
      m_vertexBuffer.allocate(
         /* data = */ m_rawVertices.constData(),
         /* count = */ m_rawVertices.size() * 3 * sizeof(GLfloat));

      m_shader.bind();
      m_vertexBuffer.bind();

      m_shader.enableAttributeArray("vertex");
      m_shader.setAttributeBuffer(
         /* name = */ "vertex",
         /* type = */ GL_FLOAT,
         /* offset = */ 0,
         /* tupleSize = */ 3);

      m_vertexBuffer.release();
      m_shader.release();
      m_VAO.release();

      return true;
   }

   bool Line::InitializeColorBuffers()
   {
      if (!m_VAO.isCreated())
      {
         m_VAO.create();
      }

      m_VAO.bind();

      m_colorBuffer.create();
      m_colorBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
      m_colorBuffer.bind();
      m_colorBuffer.allocate(
         /* data = */ m_rawColors.constData(),
         /* count = */ m_rawColors.size() * 3 * sizeof(GLfloat));

      m_colorBuffer.bind();
      m_shader.enableAttributeArray("color");
      m_shader.setAttributeBuffer(
         /* name = */ "color",
         /* type = */ GL_FLOAT,
         /* offset = */ 0,
        /* tupleSize = */ 3);

      m_colorBuffer.release();
      m_VAO.release();

      return true;
   }

   bool Line::Render(
      const Camera& camera,
      const std::vector<Light>&,
      const Settings::Manager&)
   {
      if (!m_shouldRender)
      {
         return false;
      }

      m_shader.bind();
      m_shader.setUniformValue("mvpMatrix", camera.GetProjectionViewMatrix());

      m_VAO.bind();

      m_openGL.glDrawArrays(
         /* mode = */ GL_LINES,
         /* first = */ 0,
         /* count = */ m_rawVertices.size());

      m_shader.release();
      m_VAO.release();

      return true;
   }

   bool Line::Refresh()
   {
      InitializeVertexBuffers();
      InitializeColorBuffers();

      return true;
   }
}
