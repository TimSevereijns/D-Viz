#include "visualizationAsset.h"

#include "../DataStructs/vizNode.h"
#include "../tree.h"
#include "../Visualizations/visualization.h"

namespace
{
   /**
    * @brief SetUniformLights is a helper function to easily set all values of the GLSL defined
    * struct.
    *
    * @param[in] lights             Vector of lights to be loaded into the shader program.
    * @param[in] settings           Additional scene rendering settings.
    * @param[out] shader            The shader program to load the light data into.
    */
   void SetUniformLights(
      const std::vector<Light>& lights,
      const OptionsManager& settings,
      QOpenGLShaderProgram& shader)
   {
      for (size_t i = 0; i < lights.size(); i++)
      {
         const auto index = std::to_string(i);

         std::string position{"allLights["};
         position.append(index).append("].position");

         std::string intensity{"allLights["};
         intensity.append(index).append("].intensity");

         std::string attenuation{"allLights["};
         attenuation.append(index).append("].attenuation");

         std::string ambientCoefficient{"allLights["};
         ambientCoefficient.append(index).append("].ambientCoefficient");

         shader.setUniformValue(position.c_str(), lights[i].position);
         shader.setUniformValue(intensity.c_str(), lights[i].intensity);
         shader.setUniformValue(attenuation.c_str(), settings.m_lightAttenuationFactor);
         shader.setUniformValue(ambientCoefficient.c_str(), settings.m_ambientCoefficient);
      }
   }
}

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

bool VisualizationAsset::PrepareColorBuffers(const Camera&)
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

   m_shader.enableAttributeArray("color");
   m_shader.setAttributeBuffer("color", GL_FLOAT, /* offset = */ 0, /* tupleSize = */ 3);

   m_colorBuffer.release();
   m_VAO.release();

   return true;
}

bool VisualizationAsset::Render(
   const Camera& camera,
   const std::vector<Light>& lights,
   const OptionsManager& settings)
{
   const static QMatrix4x4 DEFAULT_MATRIX = QMatrix4x4{};

   if (!IsAssetLoaded())
   {
      return true;
   }

   m_shader.bind();
   m_shader.setUniformValue("model", DEFAULT_MATRIX);
   m_shader.setUniformValue("mvpMatrix", camera.GetProjectionViewMatrix());
   m_shader.setUniformValue("cameraPosition", camera.GetPosition());
   m_shader.setUniformValue("materialShininess", settings.m_materialShininess);

   SetUniformLights(lights, settings, m_shader);

   const QVector3D specularColor
   {
      settings.m_redLightComponent,
      settings.m_greenLightComponent,
      settings.m_blueLightComponent
   };

   m_shader.setUniformValue("materialSpecularColor", specularColor);

   m_VAO.bind();

   m_graphicsDevice.glDrawArrays(GL_TRIANGLES, /* first = */ 0, /* count = */ m_rawVertices.size());

   m_shader.release();
   m_VAO.release();

   return true;
}

bool VisualizationAsset::Reload(const Camera& camera)
{
   PrepareVertexBuffers(camera);
   PrepareColorBuffers(camera);

   return true;
}

void VisualizationAsset::UpdateVBO(const TreeNode<VizNode>& node)
{
   // @todo const -> constexpr
   const int tupleSize = 3 * sizeof(GLfloat);
   const int offsetIntoVertexBuffer = node->offsetIntoVBO * tupleSize;

   // We have to divide by two, because there's a vertex plus a normal for every color:
   const int offsetIntoColorBuffer = offsetIntoVertexBuffer / 2;

   const auto newColor = Visualization::CreateHighlightColors();

   assert(m_VAO.isCreated());
   assert(m_colorBuffer.isCreated());

   m_VAO.bind();
   m_colorBuffer.bind();

   assert(m_colorBuffer.size() >= offsetIntoColorBuffer / (3 * sizeof(GLfloat)));

   m_graphicsDevice.glBufferSubData(
      GL_ARRAY_BUFFER,
      offsetIntoColorBuffer,
      newColor.size() * tupleSize,
      newColor.constData());

   m_colorBuffer.release();
   m_VAO.release();
}
