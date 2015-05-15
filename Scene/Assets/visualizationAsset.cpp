#include "visualizationAsset.h"

VisualizationAsset::VisualizationAsset()
   : SceneAsset()
{
}

VisualizationAsset::~VisualizationAsset()
{
}

bool VisualizationAsset::PrepareVertexBuffers(const Camera& camera)
{
   m_VAO.create();
   m_VAO.bind();

   m_vertexBuffer.create();
   m_vertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
   m_vertexBuffer.bind();
   m_vertexBuffer.allocate(m_rawVertices.constData(), m_rawVertices.size() * 3 * sizeof(GLfloat));

   m_shader.bind();
   m_shader.setUniformValue("mvpMatrix", camera.GetMatrix());

   m_vertexBuffer.bind();

   m_shader.enableAttributeArray("normal");
   m_shader.setAttributeBuffer("normal", GL_FLOAT,
      /* offset = */ 3 * sizeof(GLfloat), /* tupleSize = */ 3, /* stride = */ 6 * sizeof(GLfloat));

   m_shader.enableAttributeArray("vertex");
   m_shader.setAttributeBuffer("vertex", GL_FLOAT, /* offset = */ 0,
      /* tupleSize = */ 3, /* stride = */ 6 * sizeof(GLfloat));

   m_vertexBuffer.release();
   m_shader.release();
   m_VAO.release();

   return true;
}

bool VisualizationAsset::PrepareColorBuffers(const Camera& camera)
{
   m_VAO.create();
   m_VAO.bind();

   m_colorBuffer.create();
   m_colorBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
   m_colorBuffer.bind();
   m_colorBuffer.allocate(m_rawColors.constData(), m_rawColors.size() * 3 * sizeof(GLfloat));

   m_shader.bind();
   m_shader.setUniformValue("mvpMatrix", camera.GetMatrix());

   m_colorBuffer.bind();

   m_shader.enableAttributeArray("color");
   m_shader.setAttributeBuffer("color", GL_FLOAT, /* offset = */ 0, /* tupleSize = */ 3);

   m_colorBuffer.release();
   m_shader.release();
   m_VAO.release();

   return true;
}

bool VisualizationAsset::Render(const Camera& camera)
{
   const static QMatrix4x4 DEFAULT_MATRIX = QMatrix4x4();

   m_shader.bind();
   m_shader.setUniformValue("model", DEFAULT_MATRIX);
   m_shader.setUniformValue("mvpMatrix", camera.GetMatrix());
   m_shader.setUniformValue("cameraPosition", camera.GetPosition());

   m_shader.setUniformValue("materialShininess", 80.0f);
   m_shader.setUniformValue("materialSpecularColor", QVector3D(1.0f, 1.0f, 1.0f));

   m_shader.setUniformValue("light.position", camera.GetPosition());
   m_shader.setUniformValue("light.intensity", QVector3D(1.0f, 1.0f, 1.0f));
   m_shader.setUniformValue("light.attenuation", 0.05f);
   m_shader.setUniformValue("light.ambientCoefficient", 0.01f);

   m_VAO.bind();

   glDrawArrays(GL_TRIANGLES, /* first = */ 0, /* count = */ m_rawVertices.size());

   m_shader.release();
   m_VAO.release();

   return true;
}

