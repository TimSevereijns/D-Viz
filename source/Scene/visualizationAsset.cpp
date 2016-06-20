#include "visualizationAsset.h"

#include "../DataStructs/vizNode.h"
#include "../ThirdParty/Tree.hpp"
#include "../Utilities/colorGradient.hpp"
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
         const auto indexString = std::to_string(i);

         std::string position{ "allLights[" };
         position.append(indexString).append("].position");

         std::string intensity{ "allLights[" };
         intensity.append(indexString).append("].intensity");

         std::string attenuation{ "allLights[" };
         attenuation.append(indexString).append("].attenuation");

         std::string ambientCoefficient{ "allLights[" };
         ambientCoefficient.append(indexString).append("].ambientCoefficient");

         shader.setUniformValue(position.c_str(), lights[i].position);
         shader.setUniformValue(intensity.c_str(), lights[i].intensity);
         shader.setUniformValue(attenuation.c_str(), settings.m_lightAttenuationFactor);
         shader.setUniformValue(ambientCoefficient.c_str(), settings.m_ambientCoefficient);
      }
   }

   /**
    * @brief Restores the previously selected node to its non-selected color based on the rendering
    * settings.
    *
    * @param[in] node               The node whose color needs to be restored.
    * @param[in] params             The rendering settings that will determine the color.
    *
    * @returns The color to restore the node to.
    */
   QVector<QVector3D> RestoreColor(
      const TreeNode<VizNode>& node,
      const VisualizationParameters& params)
   {
      if (node.GetData().file.type != FileType::DIRECTORY)
      {
         return Visualization::CreateFileColors();
      }

      if (!params.useDirectoryGradient)
      {
         return Visualization::CreateDirectoryColors();
      }

      auto* rootNode = &node;
      while (rootNode->GetParent())
      {
         rootNode = rootNode->GetParent();
      }

      const auto ratio =
         static_cast<float>(node->file.size) / static_cast<float>((*rootNode)->file.size);

      ColorGradient gradient;
      const auto nodeColor = gradient.GetColorAtValue(ratio);

      QVector<QVector3D> blockColors;
      blockColors.reserve(Block::VERTICES_PER_BLOCK);
      for (int i = 0; i < Block::VERTICES_PER_BLOCK; i++)
      {
         blockColors << nodeColor;
      }

      return blockColors;
   }
}

VisualizationAsset::VisualizationAsset(GraphicsDevice& device) :
   SceneAsset{ device }
{
}

bool VisualizationAsset::LoadShaders()
{
   return SceneAsset::LoadShaders("visualizationVertexShader", "visualizationFragmentShader");
}

bool VisualizationAsset::Initialize(const Camera&)
{
   const bool unitBlockInitialized = InitializeUnitBlock();
   const bool colorsInitialized = InitializeColors();
   const bool transformationsInitialized = InitializeBlockTransformations();

   const bool overallSuccess =
      unitBlockInitialized
      && colorsInitialized
      && transformationsInitialized;

   assert(overallSuccess);
   return overallSuccess;
}

bool VisualizationAsset::InitializeUnitBlock()
{
   if (!m_VAO.isCreated())
   {
      m_VAO.create();
   }

   const auto unitBlock = Block
   {
      DoublePoint3D{ 0.0, 0.0, 0.0 },
      1.0,
      1.0,
      1.0
   };

   m_unitBlockVertices.clear();

   // @todo Wrap this in a function to be placed on the Block class.
   std::for_each(std::begin(unitBlock), std::end(unitBlock),
      [&] (const auto& face)
   {
      m_unitBlockVertices << face.vertices;
   });

   m_VAO.bind();

   m_unitBlockBuffer.create();
   m_unitBlockBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
   m_unitBlockBuffer.bind();
   m_unitBlockBuffer.allocate(
      /* data = */ m_unitBlockVertices.constData(),
      /* count = */ m_unitBlockVertices.size() * 3 * sizeof(GLfloat));

   m_unitBlockBuffer.bind();

   m_shader.enableAttributeArray("vertex");
   m_shader.setAttributeBuffer(
      /* location = */ "vertex",
      /* type = */ GL_FLOAT,
      /* offset = */ 0,
      /* tupleSize = */ 3,
      /* stride = */ 6 * sizeof(GLfloat));

   m_shader.enableAttributeArray("normal");
   m_shader.setAttributeBuffer(
      /* location = */ "normal",
      /* type = */ GL_FLOAT,
      /* offset = */ 3 * sizeof(GLfloat),
      /* tupleSize = */ 3,
      /* stride = */ 6 * sizeof(GLfloat));

   m_unitBlockBuffer.release();
   m_shader.release();
   m_VAO.release();

   return true;
}

bool VisualizationAsset::InitializeColors()
{
   if (!m_VAO.isCreated())
   {
      m_VAO.create();
   }

   m_VAO.bind();

   // @todo FOR DEBUG USE
   m_blockColors = Visualization::CreateDirectoryColors();

   m_blockColorBuffer.create();
   m_blockColorBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
   m_blockColorBuffer.bind();
   m_blockColorBuffer.allocate(
      /* data = */ m_blockColors.constData(),
      /* count = */ m_blockColors.size() * 3 * sizeof(GLfloat));

   m_shader.enableAttributeArray("color");
   m_shader.setAttributeBuffer(
      /* location = */"color",
      /* type = */ GL_FLOAT,
      /* offset = */ 0,
      /* tupleSize = */ 3);

   m_blockColorBuffer.release();
   m_VAO.release();

   return true;
}

bool VisualizationAsset::InitializeBlockTransformations()
{
   return true;
}

bool VisualizationAsset::Render(
   const Camera& camera,
   const std::vector<Light>& lights,
   const OptionsManager& settings)
{
   const static auto DEFAULT_MATRIX = QMatrix4x4{ };

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

//   m_graphicsDevice.glDrawArrays(
//      /* mode = */ GL_TRIANGLES,
//      /* first = */ 0,
//      /* count = */ m_unitBlockVertices.size());

   m_graphicsDevice.glDrawArraysInstanced(
      GL_TRIANGLES,
      0,
      m_unitBlockVertices.size(),
      1
   );

   m_shader.release();
   m_VAO.release();

   return true;
}

bool VisualizationAsset::Reload(const Camera&)
{
   InitializeUnitBlock();
   InitializeColors();
   InitializeBlockTransformations();

   return true;
}

void VisualizationAsset::UpdateVBO(
   const TreeNode<VizNode>& node,
   SceneAsset::UpdateAction action,
   const VisualizationParameters& options)
{
   constexpr auto tupleSize = 3 * sizeof(GLfloat);
   const auto offsetIntoVertexBuffer = node->offsetIntoVBO * tupleSize;

   // We have to divide by two, because there's a vertex plus a normal for every color:
   const auto offsetIntoColorBuffer = offsetIntoVertexBuffer / 2;

   const auto newColor = (action == SceneAsset::UpdateAction::DESELECT)
      ? RestoreColor(node, options)
      : Visualization::CreateHighlightColors();

   assert(m_VAO.isCreated());
   assert(m_colorBuffer.isCreated());

   m_VAO.bind();
   m_colorBuffer.bind();

   assert(m_colorBuffer.size() >= offsetIntoColorBuffer / (3 * sizeof(GLfloat)));

   m_graphicsDevice.glBufferSubData(
      /* target = */ GL_ARRAY_BUFFER,
      /* offset = */ offsetIntoColorBuffer,
      /* size = */ newColor.size() * tupleSize,
      /* data = */ newColor.constData());

   m_colorBuffer.release();
   m_VAO.release();
}
