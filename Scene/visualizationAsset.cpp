#include "visualizationAsset.h"

VisualizationAsset::VisualizationAsset(GraphicsDevice& device)
   : SceneAsset(device)
{
}

bool VisualizationAsset::LoadShaders()
{
   return SceneAsset::LoadShaders("visualizationVertexShader", "visualizationFragmentShader");
}

bool VisualizationAsset::PrepareVertexBuffers(const Camera& camera)
{
   m_VAO.create();  ///< This is a bit a hack: I happen to know that I load vertices before colors
   m_VAO.bind();

   m_vertexBuffer.create();
   m_vertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
   m_vertexBuffer.bind();
   m_vertexBuffer.allocate(m_rawVertices.constData(), m_rawVertices.size() * 3 * sizeof(GLfloat));

   m_shader.bind();
   m_shader.setUniformValue("mvpMatrix", camera.GetProjectionViewMatrix());

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

bool VisualizationAsset::Render(const Camera& camera, const Light& light,
   const OptionsManager& settings)
{
   const static QMatrix4x4 DEFAULT_MATRIX = QMatrix4x4();

   if (!IsAssetLoaded())
   {
      return true;
   }

   m_shader.bind();
   m_shader.setUniformValue("model",                     DEFAULT_MATRIX);
   m_shader.setUniformValue("mvpMatrix",                 camera.GetProjectionViewMatrix());
   m_shader.setUniformValue("cameraPosition",            camera.GetPosition());

   m_shader.setUniformValue("materialShininess",         settings.m_materialShininess);

   const QVector3D specularColor
   {
      settings.m_redLightComponent,
      settings.m_greenLightComponent,
      settings.m_blueLightComponent
   };

   m_shader.setUniformValue("materialSpecularColor",     specularColor);

   m_shader.setUniformValue("light.position",            light.position);
   m_shader.setUniformValue("light.intensity",           light.intensity);
   m_shader.setUniformValue("light.attenuation",         settings.m_lightAttenuationFactor);
   m_shader.setUniformValue("light.ambientCoefficient",  settings.m_ambientCoefficient);

   m_VAO.bind();

   m_graphicsDevice.glDrawArrays(GL_TRIANGLES, /* first = */ 0, /* count = */ m_rawVertices.size());

   m_shader.release();
   m_VAO.release();

   return true;
}

bool VisualizationAsset::Reload(const Camera& camera)
{
   m_shader.removeAllShaders();
   m_VAO.destroy();

   PrepareVertexBuffers(camera);
   PrepareColorBuffers(camera);

   LoadShaders();

   return true;
}

