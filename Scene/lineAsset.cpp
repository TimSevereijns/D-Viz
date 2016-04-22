#include "lineAsset.h"

LineAsset::LineAsset(GraphicsDevice& device) :
   SceneAsset{ device }
{
}

bool LineAsset::LoadShaders()
{
   return SceneAsset::LoadShaders("simpleLineVertexShader", "simpleLineFragmentShader");
}

bool LineAsset::PrepareVertexBuffers(const Camera& camera)
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
   m_shader.setUniformValue("mvpMatrix", camera.GetProjectionViewMatrix());

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

bool LineAsset::PrepareColorBuffers(const Camera&)
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

bool LineAsset::Render(
   const Camera& camera,
   const std::vector<Light>&,
   const OptionsManager&)
{
   m_shader.bind();
   m_shader.setUniformValue("mvpMatrix", camera.GetProjectionViewMatrix());

   m_VAO.bind();

   m_graphicsDevice.glDrawArrays(
      /* mode = */ GL_LINES,
      /* first = */ 0,
      /* count = */ m_rawVertices.size());

   m_shader.release();
   m_VAO.release();

   return true;
}

bool LineAsset::Reload(const Camera& camera)
{
   PrepareVertexBuffers(camera);
   PrepareColorBuffers(camera);

   return true;
}
