#include "visualizationAsset.h"

#include "../constants.h"
#include "../DataStructs/vizNode.h"
#include "../ThirdParty/Tree.hpp"
#include "../Utilities/colorGradient.hpp"
#include "../Utilities/scopeExit.hpp"
#include "../Visualizations/visualization.h"

#include <iostream>

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
   QVector3D RestoreColor(
      const TreeNode<VizNode>& node,
      const VisualizationParameters& params)
   {
      if (node.GetData().file.type != FileType::DIRECTORY)
      {
         return Constants::Colors::FILE_GREEN;
      }

      if (!params.useDirectoryGradient)
      {
         return Constants::Colors::WHITE;
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
      return nodeColor;
   }

   /**
    * @brief ComputeLightTransformationMatrix
    *
    * @param camera
    *
    * @returns
    */
   QMatrix4x4 ComputeLightTransformationMatrix(const Camera& camera)
   {
      Camera shadowCam = camera;
      shadowCam.SetPosition(QVector3D{ -200.0f, 250.0f, 200.0f });
      shadowCam.SetOrientation(10.0f, 45.0f);
      return shadowCam.GetProjectionViewMatrix();
   }

    static const QMatrix4x4 biasMatrix
    {
        0.5f, 0.0f, 0.0f, 0.5f,
        0.0f, 0.5f, 0.0f, 0.5f,
        0.0f, 0.0f, 0.5f, 0.5f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    bool shouldUpdateDepthTexture = true;
}

VisualizationAsset::VisualizationAsset(GraphicsDevice& device) :
   SceneAsset{ device }
{
}

bool VisualizationAsset::LoadShaders()
{
   m_shadowShader.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Shaders/shadowMapping.vert");
   m_shadowShader.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Shaders/shadowMapping.frag");
   bool success = m_shadowShader.link();

   success &= SceneAsset::LoadShaders("visualizationVertexShader", "visualizationFragmentShader");

   return success;
}

bool VisualizationAsset::Initialize()
{
   const bool unitBlockInitialized = InitializeReferenceBlock();
   const bool transformationsInitialized = InitializeBlockTransformations();
   const bool colorsInitialized = InitializeColors();
   const bool shadowMachineryInitialized = InitializeShadowMachinery();

   const bool overallSuccess =
      unitBlockInitialized
      && transformationsInitialized
      && colorsInitialized
      && shadowMachineryInitialized;

   assert(overallSuccess);
   return overallSuccess;
}

bool VisualizationAsset::InitializeReferenceBlock()
{
   if (!m_VAO.isCreated())
   {
      m_VAO.create();
   }

   const auto referenceBlock = Block
   {
      PrecisePoint{ 0.0, 0.0, 0.0 },
      1.0,
      1.0,
      1.0,
      /* generateVertices = */ true
   };

   m_referenceBlockVertices.clear();
   m_referenceBlockVertices = referenceBlock.GetVerticesAndNormals();

   m_VAO.bind();

   m_referenceBlockBuffer.create();
   m_referenceBlockBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
   m_referenceBlockBuffer.bind();
   m_referenceBlockBuffer.allocate(
      /* data = */ m_referenceBlockVertices.constData(),
      /* count = */ m_referenceBlockVertices.size() * sizeof(QVector3D));

   m_referenceBlockBuffer.bind();

   m_shader.enableAttributeArray("vertex");
   m_shader.setAttributeBuffer(
      /* location = */ "vertex",
      /* type = */ GL_FLOAT,
      /* offset = */ 0,
      /* tupleSize = */ 3,
      /* stride = */ 2 * sizeof(QVector3D));

   m_shader.enableAttributeArray("normal");
   m_shader.setAttributeBuffer(
      /* location = */ "normal",
      /* type = */ GL_FLOAT,
      /* offset = */ sizeof(QVector3D),
      /* tupleSize = */ 3,
      /* stride = */ 2 * sizeof(QVector3D));

   m_referenceBlockBuffer.release();
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

   m_blockColorBuffer.create();
   m_blockColorBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
   m_blockColorBuffer.bind();
   m_blockColorBuffer.allocate(
      /* data = */ m_blockColors.constData(),
      /* count = */ m_blockColors.size() * 3 * sizeof(GLfloat));

   m_graphicsDevice.glEnableVertexAttribArray(0);
   m_graphicsDevice.glVertexAttribDivisor(0, 1);
   m_graphicsDevice.glVertexAttribPointer(
      /* indx = */ 0,
      /* size = */ 3,
      /* type = */ GL_FLOAT,
      /* normalized = */ GL_FALSE,
      /* stride = */ sizeof(QVector3D),
      /* ptr = */ (GLvoid*)0);

   m_blockColorBuffer.release();
   m_VAO.release();

   return true;
}

bool VisualizationAsset::InitializeBlockTransformations()
{
   if (!m_VAO.isCreated())
   {
      m_VAO.create();
   }

   m_VAO.bind();

   constexpr auto sizeOfVector = sizeof(QVector4D);
   constexpr auto sizeOfMatrix = sizeof(QMatrix4x4);

   m_blockTransformationBuffer.create();
   m_blockTransformationBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
   m_blockTransformationBuffer.bind();
   m_blockTransformationBuffer.allocate(
      /* data = */ m_blockTransformations.constData(),
      /* count = */ m_blockTransformations.size() * sizeOfMatrix);

   // Row 1 of the matrix:
   m_graphicsDevice.glEnableVertexAttribArray(1);
   m_graphicsDevice.glVertexAttribDivisor(1, 1);
   m_graphicsDevice.glVertexAttribPointer(
      /* indx = */ 1,
      /* size = */ 4,
      /* type = */ GL_FLOAT,
      /* normalized = */ GL_FALSE,
      /* stride = */ sizeOfMatrix,
      /* ptr = */ (GLvoid*)(0 * sizeOfVector));

   // Row 2 of the matrix:
   m_graphicsDevice.glEnableVertexAttribArray(2);
   m_graphicsDevice.glVertexAttribDivisor(2, 1);
   m_graphicsDevice.glVertexAttribPointer(
      /* indx = */ 2,
      /* size = */ 4,
      /* type = */ GL_FLOAT,
      /* normalized = */ GL_FALSE,
      /* stride = */ sizeOfMatrix,
      /* ptr = */ (GLvoid*)(1 * sizeOfVector));

   // Row 3 of the matrix:
   m_graphicsDevice.glEnableVertexAttribArray(3);
   m_graphicsDevice.glVertexAttribDivisor(3, 1);
   m_graphicsDevice.glVertexAttribPointer(
      /* indx = */ 3,
      /* size = */ 4,
      /* type = */ GL_FLOAT,
      /* normalized = */ GL_FALSE,
      /* stride = */ sizeOfMatrix,
      /* ptr = */ (GLvoid*)(2 * sizeOfVector));

   // Row 4 of the matrix:
   m_graphicsDevice.glEnableVertexAttribArray(4);
   m_graphicsDevice.glVertexAttribDivisor(4, 1);
   m_graphicsDevice.glVertexAttribPointer(
      /* indx = */ 4,
      /* size = */ 4,
      /* type = */ GL_FLOAT,
      /* normalized = */ GL_FALSE,
      /* stride = */ sizeOfMatrix,
      /* ptr = */ (GLvoid*)(3 * sizeOfVector));

   m_blockTransformationBuffer.release();
   m_VAO.release();

   return true;
}

bool VisualizationAsset::InitializeShadowMachinery()
{
   m_VAO.bind();
   m_referenceBlockBuffer.bind();
   m_shadowShader.bind();

   m_shadowShader.enableAttributeArray("vertex");
   m_shadowShader.setAttributeBuffer(
      /* location = */ "vertex",
      /* type = */ GL_FLOAT,
      /* offset = */ 0,
      /* tupleSize = */ 3,
      /* stride = */ 2 * sizeof(QVector3D));

   m_shadowShader.enableAttributeArray("normal");
   m_shadowShader.setAttributeBuffer(
      /* location = */ "normal",
      /* type = */ GL_FLOAT,
      /* offset = */ sizeof(QVector3D),
      /* tupleSize = */ 3,
      /* stride = */ 2 * sizeof(QVector3D));

   m_shadowShader.release();
   m_referenceBlockBuffer.release();
   m_VAO.release();

   m_graphicsDevice.glGenFramebuffers(1, &m_shadowMapFrameBufferID);

   m_graphicsDevice.glGenTextures(1, &m_shadowMapTextureID);
   m_graphicsDevice.glBindTexture(GL_TEXTURE_2D, m_shadowMapTextureID);

   m_graphicsDevice.glTexImage2D(
      GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_MAP_WIDTH,
      SHADOW_MAP_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

   m_graphicsDevice.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   m_graphicsDevice.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   m_graphicsDevice.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
   m_graphicsDevice.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

   constexpr GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
   m_graphicsDevice.glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

   m_graphicsDevice.glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMapFrameBufferID);

   m_graphicsDevice.glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowMapTextureID, 0);

   m_graphicsDevice.glDrawBuffer(GL_NONE);
   m_graphicsDevice.glReadBuffer(GL_NONE);

   m_graphicsDevice.glBindFramebuffer(GL_FRAMEBUFFER, 0);

   return true;
}

std::uint32_t VisualizationAsset::LoadBufferData(
   const Tree<VizNode>& tree,
   const VisualizationParameters& parameters)
{
   m_blockTransformations.clear();
   m_blockColors.clear();

   m_blockCount = 0;

   for (auto&& node : tree)
   {
      const bool fileIsTooSmall = (node->file.size < parameters.minimumFileSize);
      const bool notTheRightFileType =
         parameters.onlyShowDirectories && node->file.type != FileType::DIRECTORY;

      if (notTheRightFileType || fileIsTooSmall)
      {
         continue;
      }

      node->offsetIntoVBO = m_blockCount++;

      const auto& block = node->block;

      QMatrix4x4 instanceMatrix{ };
      instanceMatrix.translate(block.GetOrigin().x(), block.GetOrigin().y(), block.GetOrigin().z());
      instanceMatrix.scale(block.GetWidth(), block.GetHeight(), block.GetDepth());
      m_blockTransformations << instanceMatrix;

      if (node->file.type == FileType::DIRECTORY)
      {
         if (parameters.useDirectoryGradient)
         {
            m_blockColors << ComputeGradientColor(node);
         }
         else
         {
            m_blockColors << Constants::Colors::WHITE;
         }
      }
      else if (node->file.type == FileType::REGULAR)
      {
         m_blockColors << Constants::Colors::FILE_GREEN;
      }
   }

   FindLargestDirectory(tree);

   assert(m_blockColors.size() == m_blockTransformations.size());
   assert(m_blockColors.size() == m_blockCount);

   return m_blockCount;
}

void VisualizationAsset::FindLargestDirectory(const Tree<VizNode>& tree)
{
   std::uintmax_t largestDirectory = std::numeric_limits<std::uintmax_t>::min();

   for (auto&& node : tree)
   {
      if (node.GetData().file.type != FileType::DIRECTORY)
      {
         continue;
      }

      const auto directorySize = node.GetData().file.size;

      if (directorySize > largestDirectory)
      {
         largestDirectory = directorySize;
      }
   }

   m_largestDirectorySize = largestDirectory;
}

QVector3D VisualizationAsset::ComputeGradientColor(const TreeNode<VizNode>& node)
{
   const auto blockSize = node.GetData().file.size;
   const auto ratio = static_cast<double>(blockSize) / static_cast<double>(m_largestDirectorySize);

   const auto finalColor = m_directoryColorGradient.GetColorAtValue(static_cast<float>(ratio));
   return finalColor;
}

std::uint32_t VisualizationAsset::GetBlockCount() const
{
   return m_blockCount;
}

bool VisualizationAsset::IsAssetLoaded() const
{
   return !(m_blockTransformations.empty() && m_blockColors.empty());
}

bool VisualizationAsset::RenderShadowPass(const Camera& camera)
{
   ON_SCOPE_EXIT
   {
      const auto& viewport = camera.GetViewport();
      m_graphicsDevice.glViewport(0, 0, viewport.width(), viewport.height());
   };

   // Since Qt uses at least one framebuffer for its own rendering needs, we'll need to
   // save and then restore the current framebuffer:
   GLint qtBuffer;
   m_graphicsDevice.glGetIntegerv(GL_FRAMEBUFFER_BINDING, &qtBuffer);

   m_graphicsDevice.glViewport(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);

   m_graphicsDevice.glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMapFrameBufferID);
   m_graphicsDevice.glClear(GL_DEPTH_BUFFER_BIT);

   m_shadowShader.bind();

   m_shadowShader.setUniformValue("cameraProjectionViewMatrix",
      ComputeLightTransformationMatrix(camera));

   m_VAO.bind();

   m_graphicsDevice.glDrawArraysInstanced(
      /* mode = */ GL_TRIANGLES,
      /* first = */ 0,
      /* count = */ m_referenceBlockVertices.size(),
      /* instanceCount = */ m_blockColors.size()
   );

   m_VAO.release();

   m_shadowShader.release();

   m_graphicsDevice.glBindFramebuffer(GL_FRAMEBUFFER, qtBuffer);

   return true;
}

bool VisualizationAsset::RenderMainPass(
   const Camera& camera,
   const std::vector<Light>& lights,
   const OptionsManager& settings)
{
   m_graphicsDevice.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   m_shader.bind();

   m_shader.setUniformValue("cameraPosition", camera.GetPosition());
   m_shader.setUniformValue("materialShininess", settings.m_materialShininess);

   const QMatrix4x4 shadowProjectionViewMatrix =
      biasMatrix * ComputeLightTransformationMatrix(camera);

   m_shader.setUniformValue("shadowProjectionViewMatrix", shadowProjectionViewMatrix );
   m_shader.setUniformValue("cameraProjectionViewMatrix", camera.GetProjectionViewMatrix());

   SetUniformLights(lights, settings, m_shader);

   // @todo No need to set this repeatedly now that the color isn't controlled by the UI.
   m_shader.setUniformValue("materialSpecularColor", QVector3D{ 0.0f, 0.0f, 0.0f });

   m_graphicsDevice.glActiveTexture(GL_TEXTURE0);
   m_graphicsDevice.glBindTexture(GL_TEXTURE_2D, m_shadowMapTextureID);

   m_VAO.bind();

   m_graphicsDevice.glDrawArraysInstanced(
      /* mode = */ GL_TRIANGLES,
      /* first = */ 0,
      /* count = */ m_referenceBlockVertices.size(),
      /* instanceCount = */ m_blockColors.size()
   );

   m_VAO.release();

   m_shader.release();

   return true;
}

bool VisualizationAsset::Render(
   const Camera& camera,
   const std::vector<Light>& lights,
   const OptionsManager& settings)
{
   if (!IsAssetLoaded())
   {
      return true;
   }

   RenderShadowPass(camera);
   RenderMainPass(camera, lights, settings);

   return true;
}

bool VisualizationAsset::Reload()
{
   InitializeReferenceBlock();
   InitializeBlockTransformations();
   InitializeColors();

   return true;
}

void VisualizationAsset::UpdateVBO(
   const TreeNode<VizNode>& node,
   SceneAsset::UpdateAction action,
   const VisualizationParameters& options)
{
   constexpr auto colorDataTupleSize{ sizeof(QVector3D) };
   const auto offsetIntoColorBuffer = node->offsetIntoVBO * colorDataTupleSize;

   const auto newColor = (action == SceneAsset::UpdateAction::DESELECT)
      ? RestoreColor(node, options)
      : Constants::Colors::CANARY_YELLOW;

   assert(m_VAO.isCreated());
   assert(m_blockColorBuffer.isCreated());
   assert(m_blockColorBuffer.size() >= offsetIntoColorBuffer / colorDataTupleSize);

   m_VAO.bind();
   m_blockColorBuffer.bind();

   m_graphicsDevice.glBufferSubData(
      /* target = */ GL_ARRAY_BUFFER,
      /* offset = */ offsetIntoColorBuffer,
      /* size = */ colorDataTupleSize,
      /* data = */ &newColor);

   m_blockColorBuffer.release();
   m_VAO.release();
}
