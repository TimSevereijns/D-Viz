#include "treemapAsset.h"

#include "../constants.h"
#include "../DataStructs/vizFile.h"
#include "../settings.h"
#include "../Utilities/colorGradient.hpp"
#include "../Utilities/scopeExit.hpp"
#include "../Visualizations/visualization.h"

#include <boost/optional.hpp>

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
      const Settings::Manager& settings,
      QOpenGLShaderProgram& shader)
   {
      for (std::size_t i = 0; i < lights.size(); i++)
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
         shader.setUniformValue(attenuation.c_str(), settings.GetLightAttentuationFactor());
         shader.setUniformValue(ambientCoefficient.c_str(), settings.GetAmbientLightCoefficient());
      }
   }

   /**
    * @brief Determines the appropriate color for the file based on the user-configurable color set
    * in the color.json file.
    *
    * @param[in] node               The node whose color needs to be restored.
    * @param[in] settings           The settings that will determine the color.
    *
    * @returns The appropriate color found in the color map.
    */
   boost::optional<QVector3D> DetermineColorFromExtension(
      const Tree<VizFile>::Node& node,
      const Settings::Manager& settings)
   {
      const auto& colorMap = settings.GetFileColorMap();
      const auto categoryItr = colorMap.find(settings.GetActiveColorScheme());
      if (categoryItr == std::end(colorMap))
      {
         return boost::none;
      }

      const auto extensionItr = categoryItr->second.find(node->file.extension);
      if (extensionItr == std::end(categoryItr->second))
      {
         return boost::none;
      }

      return extensionItr->second;
   }

   /**
    * @brief Restores the previously selected node to its non-selected color based on the rendering
    * settings.
    *
    * @param[in] node               The node whose color needs to be restored.
    * @param[in] settings           The settings that will determine the color.
    *
    * @returns The color to restore the node to.
    */
   QVector3D RestoreColor(
      const Tree<VizFile>::Node& node,
      const Settings::Manager& settings)
   {
      const auto fileColor = DetermineColorFromExtension(node, settings);
      if (fileColor)
      {
         return *fileColor;
      }

      if (node.GetData().file.type != FileType::DIRECTORY)
      {
         return Constants::Colors::FILE_GREEN;
      }

      if (!settings.GetVisualizationParameters().useDirectoryGradient)
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

namespace Asset
{
   Treemap::Treemap(QOpenGLExtraFunctions& openGL) :
      Base{ openGL }
   {
   }

   bool Treemap::LoadShaders()
   {
      return Base::LoadShaders("visualizationVertexShader", "visualizationFragmentShader");
   }

   bool Treemap::Initialize()
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

   bool Treemap::InitializeReferenceBlock()
   {
      if (!m_VAO.isCreated())
      {
         m_VAO.create();
      }

      const auto referenceBlock = Block
      {
         PrecisePoint{ 0.0, 0.0, 0.0 },
         /* width =  */ 1.0,
         /* height = */ 1.0,
         /* depth =  */ 1.0,
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

   bool Treemap::InitializeColors()
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

      m_openGL.glEnableVertexAttribArray(0);
      m_openGL.glVertexAttribDivisor(0, 1);
      m_openGL.glVertexAttribPointer(
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

   bool Treemap::InitializeBlockTransformations()
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
      m_openGL.glEnableVertexAttribArray(1);
      m_openGL.glVertexAttribDivisor(1, 1);
      m_openGL.glVertexAttribPointer(
         /* indx = */ 1,
         /* size = */ 4,
         /* type = */ GL_FLOAT,
         /* normalized = */ GL_FALSE,
         /* stride = */ sizeOfMatrix,
         /* ptr = */ reinterpret_cast<GLvoid*>(0 * sizeOfVector));

      // Row 2 of the matrix:
      m_openGL.glEnableVertexAttribArray(2);
      m_openGL.glVertexAttribDivisor(2, 1);
      m_openGL.glVertexAttribPointer(
         /* indx = */ 2,
         /* size = */ 4,
         /* type = */ GL_FLOAT,
         /* normalized = */ GL_FALSE,
         /* stride = */ sizeOfMatrix,
         /* ptr = */ reinterpret_cast<GLvoid*>(1 * sizeOfVector));

      // Row 3 of the matrix:
      m_openGL.glEnableVertexAttribArray(3);
      m_openGL.glVertexAttribDivisor(3, 1);
      m_openGL.glVertexAttribPointer(
         /* indx = */ 3,
         /* size = */ 4,
         /* type = */ GL_FLOAT,
         /* normalized = */ GL_FALSE,
         /* stride = */ sizeOfMatrix,
         /* ptr = */ reinterpret_cast<GLvoid*>(2 * sizeOfVector));

      // Row 4 of the matrix:
      m_openGL.glEnableVertexAttribArray(4);
      m_openGL.glVertexAttribDivisor(4, 1);
      m_openGL.glVertexAttribPointer(
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

   std::uint32_t Treemap::LoadBufferData(
      const Tree<VizFile>& tree,
      const Settings::Manager& settings)
   {
      m_blockTransformations.clear();
      m_blockColors.clear();

      m_blockCount = 0;

      const auto& parameters = settings.GetVisualizationParameters();

      for (auto& node : tree)
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

         ComputeAppropriateBlockColor(node, settings);
      }

      FindLargestDirectory(tree);

      assert(m_blockColors.size() == m_blockTransformations.size());
      assert(m_blockColors.size() == static_cast<int>(m_blockCount));

      return m_blockCount;
   }

   void Treemap::ReloadColorBufferData(
      const Tree<VizFile>& tree,
      const Settings::Manager& settings)
   {
      m_blockColors.clear();

      const auto& parameters = settings.GetVisualizationParameters();

      for (const auto& node : tree)
      {
         const bool fileIsTooSmall = (node->file.size < parameters.minimumFileSize);
         const bool notTheRightFileType =
            parameters.onlyShowDirectories && node->file.type != FileType::DIRECTORY;

         if (notTheRightFileType || fileIsTooSmall)
         {
            continue;
         }

         ComputeAppropriateBlockColor(node, settings);
      }
   }

   void Treemap::FindLargestDirectory(const Tree<VizFile>& tree)
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

      assert(largestDirectory > std::numeric_limits<std::uintmax_t>::min());

      m_largestDirectorySize = largestDirectory;
   }

   QVector3D Treemap::ComputeGradientColor(const Tree<VizFile>::Node& node)
   {
      const auto blockSize = node.GetData().file.size;
      const auto ratio =
         static_cast<long double>(blockSize) / static_cast<long double>(m_largestDirectorySize);

      const auto finalColor = m_directoryColorGradient.GetColorAtValue(static_cast<float>(ratio));
      return finalColor;
   }

   void Treemap::ComputeAppropriateBlockColor(
      const Tree<VizFile>::Node& node,
      const Settings::Manager& settings)
   {
      // @todo Perform the color map lookup conditionally.

      const auto fileColor = DetermineColorFromExtension(node, settings);
      if (fileColor)
      {
         m_blockColors << *fileColor;
         return;
      }

      if (node->file.type == FileType::DIRECTORY)
      {
         if (settings.GetVisualizationParameters().useDirectoryGradient)
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

   std::uint32_t Treemap::GetBlockCount() const
   {
      return m_blockCount;
   }

   bool Treemap::IsAssetLoaded() const
   {
      return !(m_blockTransformations.empty() && m_blockColors.empty());
   }

   bool Treemap::Render(
      const Camera& camera,
      const std::vector<Light>& lights,
      const Settings::Manager& settings)
   {
      if (!IsAssetLoaded())
      {
         return true;
      }

      m_shader.bind();
      m_shader.setUniformValue("projectionMatrix", camera.GetProjectionMatrix());
      m_shader.setUniformValue("viewMatrix", camera.GetViewMatrix());

      m_shader.setUniformValue("cameraPosition", camera.GetPosition());
      m_shader.setUniformValue("materialShininess", settings.GetMaterialShininess());

      SetUniformLights(lights, settings, m_shader);

      // @todo No need to set this repeatedly now that the color isn't controlled by the UI.
      const QVector3D specularColor{ 0.0f, 0.0f, 0.0f };
      m_shader.setUniformValue("materialSpecularColor", specularColor);

      m_VAO.bind();

      m_openGL.glDrawArraysInstanced(
         /* mode = */ GL_TRIANGLES,
         /* first = */ 0,
         /* count = */ m_referenceBlockVertices.size(),
         /* instanceCount = */ m_blockColors.size()
      );

      m_shader.release();
      m_VAO.release();

      return true;
   }

   bool Treemap::Refresh()
   {
      InitializeReferenceBlock();
      InitializeColors();
      InitializeBlockTransformations();

      return true;
   }

   void Treemap::UpdateVBO(
      const Tree<VizFile>::Node& node,
      Asset::Event action,
      const Settings::Manager& settings)
   {
      assert(m_VAO.isCreated());
      assert(m_blockColorBuffer.isCreated());
      assert(static_cast<int>(node->offsetIntoVBO) < m_blockCount);

      constexpr auto colorTupleSize{ sizeof(QVector3D) };
      const auto offsetIntoColorBuffer = node->offsetIntoVBO * colorTupleSize;

      const auto newColor = (action == Asset::Event::DESELECTION)
         ? RestoreColor(node, settings)
         : Constants::Colors::CANARY_YELLOW;

      m_VAO.bind();
      m_blockColorBuffer.bind();

      m_openGL.glBufferSubData(
         /* target = */ GL_ARRAY_BUFFER,
         /* offset = */ offsetIntoColorBuffer,
         /* size = */ colorTupleSize,
         /* data = */ &newColor);

      m_blockColorBuffer.release();
      m_VAO.release();
   }
}
