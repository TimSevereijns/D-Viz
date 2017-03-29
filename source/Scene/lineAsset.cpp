#include "lineAsset.h"

LineAsset::LineAsset(GraphicsDevice& device) :
   SceneAsset{ device }
{
}

bool LineAsset::LoadShaders()
{
   return SceneAsset::LoadShaders("simpleLineVertexShader", "simpleLineFragmentShader");
}

bool LineAsset::Initialize()
{
   const bool vertexBuffersLoadedSuccessfully = InitializeVertexBuffers();
   const bool colorBuffersLoadedSuccessfully = InitializeColorBuffers();

   const bool overallSuccess = vertexBuffersLoadedSuccessfully && colorBuffersLoadedSuccessfully;
   assert(overallSuccess);

   return overallSuccess;
}

bool LineAsset::InitializeVertexBuffers()
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

   return true;
}

bool LineAsset::InitializeColorBuffers()
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
   m_mainShader.enableAttributeArray("color");
   m_mainShader.setAttributeBuffer(
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
   m_mainShader.bind();
   m_mainShader.setUniformValue("mvpMatrix", camera.GetProjectionViewMatrix());

   m_VAO.bind();

   m_graphicsDevice.glDrawArrays(
      /* mode = */ GL_LINES,
      /* first = */ 0,
      /* count = */ m_rawVertices.size());

   m_mainShader.release();
   m_VAO.release();

   return true;
}

bool LineAsset::Reload()
{
   InitializeVertexBuffers();
   InitializeColorBuffers();

   return true;
}
