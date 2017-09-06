#include "treemapAsset.h"

#include "../constants.h"
#include "../DataStructs/vizFile.h"
#include "../Utilities/colorGradient.hpp"
#include "../Utilities/scopeExit.hpp"
#include "../Visualizations/visualization.h"

#include <Tree/Tree.hpp>

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
      const Tree<VizFile>::Node& node,
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
}

TreemapAsset::TreemapAsset(QOpenGLExtraFunctions& device) :
   SceneAsset{ device }
{
}

bool TreemapAsset::LoadShaders()
{
   return SceneAsset::LoadShaders("visualizationVertexShader", "visualizationFragmentShader");
}

bool TreemapAsset::Initialize()
{
   const bool unitBlockInitialized = InitializeReferenceBlock();
   const bool colorsInitialized = InitializeColors();
   const bool transformationsInitialized = InitializeBlockTransformations();

   const bool overallSuccess =
      unitBlockInitialized
      && colorsInitialized
      && transformationsInitialized;

   assert(overallSuccess);
   return overallSuccess;
}

bool TreemapAsset::InitializeReferenceBlock()
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

bool TreemapAsset::InitializeColors()
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
      /* ptr = */ static_cast<GLvoid*>(0));

   m_blockColorBuffer.release();
   m_VAO.release();

   return true;
}

bool TreemapAsset::InitializeBlockTransformations()
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
      /* ptr = */ reinterpret_cast<GLvoid*>(0 * sizeOfVector));

   // Row 2 of the matrix:
   m_graphicsDevice.glEnableVertexAttribArray(2);
   m_graphicsDevice.glVertexAttribDivisor(2, 1);
   m_graphicsDevice.glVertexAttribPointer(
      /* indx = */ 2,
      /* size = */ 4,
      /* type = */ GL_FLOAT,
      /* normalized = */ GL_FALSE,
      /* stride = */ sizeOfMatrix,
      /* ptr = */ reinterpret_cast<GLvoid*>(1 * sizeOfVector));

   // Row 3 of the matrix:
   m_graphicsDevice.glEnableVertexAttribArray(3);
   m_graphicsDevice.glVertexAttribDivisor(3, 1);
   m_graphicsDevice.glVertexAttribPointer(
      /* indx = */ 3,
      /* size = */ 4,
      /* type = */ GL_FLOAT,
      /* normalized = */ GL_FALSE,
      /* stride = */ sizeOfMatrix,
      /* ptr = */ reinterpret_cast<GLvoid*>(2 * sizeOfVector));

   // Row 4 of the matrix:
   m_graphicsDevice.glEnableVertexAttribArray(4);
   m_graphicsDevice.glVertexAttribDivisor(4, 1);
   m_graphicsDevice.glVertexAttribPointer(
      /* indx = */ 4,
      /* size = */ 4,
      /* type = */ GL_FLOAT,
      /* normalized = */ GL_FALSE,
      /* stride = */ sizeOfMatrix,
      /* ptr = */ reinterpret_cast<GLvoid*>(3 * sizeOfVector));

   m_blockTransformationBuffer.release();
   m_VAO.release();

   return true;
}

std::uint32_t TreemapAsset::LoadBufferData(
   const Tree<VizFile>& tree,
   const VisualizationParameters& parameters)
{
   m_blockTransformations.clear();
   m_blockColors.clear();

   m_blockCount = 0;

   for (Tree<VizFile>::Node& node : tree)
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
      const auto& blockOrigin = block.GetOrigin();

      QMatrix4x4 instanceMatrix{ };
      instanceMatrix.translate(blockOrigin.x(), blockOrigin.y(), blockOrigin.z());
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
   assert(m_blockColors.size() == static_cast<int>(m_blockCount));

   return m_blockCount;
}

void TreemapAsset::FindLargestDirectory(const Tree<VizFile>& tree)
{
   std::uintmax_t largestDirectory = std::numeric_limits<std::uintmax_t>::min();

   for (auto& node : tree)
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

QVector3D TreemapAsset::ComputeGradientColor(const Tree<VizFile>::Node& node)
{
   const auto blockSize = node.GetData().file.size;
   const auto ratio = static_cast<double>(blockSize) / static_cast<double>(m_largestDirectorySize);

   const auto finalColor = m_directoryColorGradient.GetColorAtValue(static_cast<float>(ratio));
   return finalColor;
}

std::uint32_t TreemapAsset::GetBlockCount() const
{
   return m_blockCount;
}

bool TreemapAsset::IsAssetLoaded() const
{
   return !(m_blockTransformations.empty() && m_blockColors.empty());
}

bool TreemapAsset::Render(
   const Camera& camera,
   const std::vector<Light>& lights,
   const OptionsManager& settings)
{
   if (!IsAssetLoaded())
   {
      return true;
   }

   m_shader.bind();
   m_shader.setUniformValue("projectionMatrix", camera.GetProjectionMatrix());
   m_shader.setUniformValue("viewMatrix", camera.GetViewMatrix());

   m_shader.setUniformValue("cameraPosition", camera.GetPosition());
   m_shader.setUniformValue("materialShininess", settings.m_materialShininess);

   SetUniformLights(lights, settings, m_shader);

   // @todo No need to set this repeatedly now that the color isn't controlled by the UI.
   const QVector3D specularColor{ 0.0f, 0.0f, 0.0f };
   m_shader.setUniformValue("materialSpecularColor", specularColor);

   m_VAO.bind();

   m_graphicsDevice.glDrawArraysInstanced(
      /* mode = */ GL_TRIANGLES,
      /* first = */ 0,
      /* count = */ m_referenceBlockVertices.size(),
      /* instanceCount = */ m_blockColors.size()
   );

   m_shader.release();
   m_VAO.release();

   return true;
}

bool TreemapAsset::Reload()
{
   InitializeReferenceBlock();
   InitializeColors();
   InitializeBlockTransformations();

   return true;
}

void TreemapAsset::UpdateVBO(
   const Tree<VizFile>::Node& node,
   SceneAsset::UpdateAction action,
   const VisualizationParameters& options)
{
   constexpr auto colorTupleSize{ sizeof(QVector3D) };
   const auto offsetIntoColorBuffer = node->offsetIntoVBO * colorTupleSize;

   const auto newColor = (action == SceneAsset::UpdateAction::DESELECT)
      ? RestoreColor(node, options)
      : Constants::Colors::CANARY_YELLOW;

   assert(m_VAO.isCreated());
   assert(m_blockColorBuffer.isCreated());

   // @todo This appears to fail, figure out why:
   //assert(m_blockColorBuffer.size() >= static_cast<int>(offsetIntoColorBuffer / colorTupleSize));

   m_VAO.bind();
   m_blockColorBuffer.bind();

   m_graphicsDevice.glBufferSubData(
      /* target = */ GL_ARRAY_BUFFER,
      /* offset = */ offsetIntoColorBuffer,
      /* size = */ colorTupleSize,
      /* data = */ &newColor);

   m_blockColorBuffer.release();
   m_VAO.release();
}