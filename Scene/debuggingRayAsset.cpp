#include "debuggingRayAsset.h"

namespace
{
   QVector<QVector3D> CreateDefaultRayVertices()
   {
      // The default location of the ray is under the visualization.
      QVector<QVector3D> vertices;
      vertices << QVector3D(500.0f, -100.0f, 0.0f) << QVector3D(500.0f, -100.0f, -1000.0f);

      return vertices;
   }

   QVector<QVector3D> CreateDefaultRayColors()
   {
      // The default color is "Hot Pink" (RGB: 255, 105, 180)
      QVector<QVector3D> colors;
      colors
         << QVector3D(1.0f, 105.0f / 255.0f, 180.0f / 255.0f)
         << QVector3D(1.0f, 105.0f / 255.0f, 180.0f / 255.0f);

      return colors;
   }
}

DebuggingRayAsset::DebuggingRayAsset(GraphicsDevice& device)
   : SceneAsset(device)
{
   m_rawVertices = CreateDefaultRayVertices();
   m_rawColors = CreateDefaultRayColors();
}

bool DebuggingRayAsset::LoadShaders()
{
   return SceneAsset::LoadShaders("originMarkerVertexShader", "originMarkerFragmentShader");
}

bool DebuggingRayAsset::PrepareVertexBuffers(const Camera& camera)
{
   if (!m_VAO.isCreated())
   {
      m_VAO.create();
   }

   m_VAO.bind();

   m_vertexBuffer.create();
   m_vertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
   m_vertexBuffer.bind();
   m_vertexBuffer.allocate(m_rawVertices.constData(), m_rawVertices.size() * 3 * sizeof(GLfloat));

   m_shader.bind();
   m_shader.setUniformValue("mvpMatrix", camera.GetProjectionViewMatrix());

   m_vertexBuffer.bind();
   m_shader.enableAttributeArray("vertex");
   m_shader.setAttributeBuffer("vertex", GL_FLOAT, /* offset = */ 0, /* tupleSize = */ 3);

   m_vertexBuffer.release();
   m_shader.release();
   m_VAO.release();

   return true;
}

bool DebuggingRayAsset::PrepareColorBuffers(const Camera& camera)
{
   if (!m_VAO.isCreated())
   {
      m_VAO.create();
   }

   m_VAO.bind();

   m_colorBuffer.create();
   m_colorBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
   m_colorBuffer.bind();
   m_colorBuffer.allocate(m_rawColors.constData(), m_rawColors.size() * 3 * sizeof(GLfloat));

   m_shader.bind();
   m_shader.setUniformValue("mvpMatrix", camera.GetProjectionViewMatrix());

   m_colorBuffer.bind();
   m_shader.enableAttributeArray("color");
   m_shader.setAttributeBuffer("color", GL_FLOAT, /* offset = */ 0, /* tupleSize = */ 3);

   m_colorBuffer.release();
   m_shader.release();
   m_VAO.release();

   return true;
}

bool DebuggingRayAsset::Render(const Camera& camera, const Light&, const OptionsManager&)
{
   m_shader.bind();
   m_shader.setUniformValue("mvpMatrix", camera.GetProjectionViewMatrix());

   m_VAO.bind();

   m_graphicsDevice.glDrawArrays(GL_LINES, /* first = */ 0, /* count = */ m_rawVertices.size());

   m_shader.release();
   m_VAO.release();

   return true;
}

bool DebuggingRayAsset::Reload(const Camera& camera)
{
   PrepareVertexBuffers(camera);

   return true;
}

void DebuggingRayAsset::SetDebuggingRay(const QVector3D& start, const QVector3D& end)
{
   m_rawVertices.clear();
   m_rawVertices.append(start);
   m_rawVertices.append(end);
}
