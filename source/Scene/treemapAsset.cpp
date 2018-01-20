#include "treemapAsset.h"

#include "../constants.h"
#include "../DataStructs/vizFile.h"
#include "../Settings/settings.h"
#include "../Utilities/colorGradient.hpp"
#include "../Utilities/scopeExit.hpp"
#include "../Visualizations/visualization.h"

#include <boost/optional.hpp>

#include <Tree/Tree.hpp>

#include <cmath>
#include <iostream>
#include <vector>

#ifdef Q_OS_WIN
   #undef near
   #undef far
#endif

namespace
{
   constexpr auto TEXTURE_PREVIEWER_VERTEX_ATTRIBUTE{ 0 };
   constexpr auto TEXTURE_PREVIEWER_TEXCOORD_ATTRIBUTE{ 1 };

   /**
    * @brief Generates all of the frustum vertices for the specified camera.
    *
    * @param[in] camera             Main scene camera.
    */
   auto ComputeFrustumCorners(const Camera& camera)
   {
      std::vector<QVector3D> unitCube
      {
         { -1, -1, -1 }, { +1, -1, -1 },
         { +1, +1, -1 }, { -1, +1, -1 },
         { -1, -1, +1 }, { +1, -1, +1 },
         { +1, +1, +1 }, { -1, +1, +1 }
      };

      const auto worldToView = camera.GetProjectionViewMatrix().inverted();
      for (auto& corner : unitCube)
      {
         corner = worldToView.map(corner);
      }

      return unitCube;
   }

   /**
    * @brief Computes the ideal split locations for each frustum cascade.
    *
    * @param[in] cascadeCount       The desired number of shadow mapping cascades.
    * @param[in] camera             The main scene camera.
    */
   auto ComputeCascadeDistances()
   {
      const std::vector<std::pair<float, float>> cascadeDistances
      {
         std::make_pair(1.0f, 25.0f),
         std::make_pair(25.0f, 100.0f),
         std::make_pair(100.0f, 500.0f),
         std::make_pair(500.0f, 2000.0f)
      };

      return cascadeDistances;
   }

   /**
    * @brief The BoundingBox struct
    */
   struct BoundingBox
   {
      float left;
      float right;
      float bottom;
      float top;
      float near;
      float far;
   };

   /**
    * @brief Calculates an Axis Aligned Bounding Box (AABB) for each of the frustum splits.
    *
    * @param[in] renderCamera       The main camera used to render the scene. Mainly use is to get
    *                               the aspect ratio of the outline correct.
    * @param[in] shadowViewMatrix   The projection-view matrix that represents the view of the
    *                               shadow casting light source.
    */
   auto ComputeFrustumSplitBoundingBoxes(
      const Camera& renderCamera,
      const QMatrix4x4& shadowViewMatrix)
   {
      static const auto cascadeDistances = ComputeCascadeDistances();

      std::vector<std::vector<QVector3D>> frusta;
      frusta.reserve(Asset::Treemap::CASCADE_COUNT);

      auto mutableCamera = renderCamera;
      for (const auto& nearAndFarPlanes : cascadeDistances)
      {
         mutableCamera.SetNearPlane(nearAndFarPlanes.first);
         mutableCamera.SetFarPlane(nearAndFarPlanes.second);

         frusta.emplace_back(ComputeFrustumCorners(mutableCamera));
      }

      std::vector<BoundingBox> boundingBoxes;
      boundingBoxes.reserve(Asset::Treemap::CASCADE_COUNT);

      const auto worldToLight = shadowViewMatrix;

      for (const auto& frustum : frusta)
      {
         auto minX = std::numeric_limits<float>::max();
         auto maxX = std::numeric_limits<float>::lowest();

         auto minY = std::numeric_limits<float>::max();
         auto maxY = std::numeric_limits<float>::lowest();

         auto minZ = std::numeric_limits<float>::max();
         auto maxZ = std::numeric_limits<float>::lowest();

         // Compute bounding box in light space:
         for (const auto& vertex : frustum)
         {
            const auto mappedVertex = worldToLight.map(vertex);

            minX = std::min(minX, mappedVertex.x());
            maxX = std::max(maxX, mappedVertex.x());

            minY = std::min(minY, mappedVertex.y());
            maxY = std::max(maxY, mappedVertex.y());

            minZ = std::min(minZ, mappedVertex.z());
            maxZ = std::max(maxZ, mappedVertex.z());
         }

         auto boundingBox = BoundingBox
         {
            /* left   = */ minX,
            /* right  = */ maxX,
            /* bottom = */ minY,
            /* top    = */ maxY,
            /* near   = */ maxZ,
            /* far    = */ minZ
         };

         boundingBoxes.emplace_back(std::move(boundingBox));
      }

      return boundingBoxes;
   }

   /**
    * @brief A helper function to set many of the shader variables needed for lighting.
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
      for (auto index{ 0u }; index < lights.size(); ++index)
      {
         const auto indexAsString = std::to_string(index);

         const auto position = "allLights[" + indexAsString + "].position";
         shader.setUniformValue(position.data(), lights[index].position);

         const auto intensity = "allLights[" + indexAsString + "].intensity";
         shader.setUniformValue(intensity.data(), lights[index].intensity);

         const auto attenuation = "allLights[" + indexAsString + "].attenuation";
         shader.setUniformValue(attenuation.data(), settings.GetLightAttentuationFactor());

         const auto ambientCoefficient = "allLights[" + indexAsString + "].ambientCoefficient";
         shader.setUniformValue(ambientCoefficient.data(), settings.GetAmbientLightCoefficient());
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
      return gradient.GetColorAtValue(ratio);
   }

   /**
    * @returns The view matrix for the shadow casting light source.
    */
   QMatrix4x4 ComputeLightViewMatrix()
   {
      constexpr auto lightPosition = QVector3D{ 0.f, 200.f, 0.f };
      constexpr auto lightTarget = QVector3D{ 500.f, 0.f, -500.f };
      constexpr auto upVector = QVector3D{ 0.0f, 1.0f, 0.0f };

      QMatrix4x4 view;
      view.lookAt(lightPosition, lightTarget, upVector);

      return view;
   }
}

namespace Asset
{
   Treemap::Treemap(
      QOpenGLExtraFunctions& openGL,
      bool isInitiallyVisible)
      :
      Base{ openGL, isInitiallyVisible }
   {
      m_shadowMaps.reserve(CASCADE_COUNT);

      for (auto index{ 0u }; index < CASCADE_COUNT; ++index)
      {
         auto frameBuffer = std::make_unique<QOpenGLFramebufferObject>
         (
            SHADOW_MAP_WIDTH,
            SHADOW_MAP_HEIGHT,
            QOpenGLFramebufferObject::Attachment::Depth,
            GL_TEXTURE_2D,
            GL_RGB32F
         );

         m_shadowMaps.emplace_back(std::move(frameBuffer));
      }
   }

   bool Treemap::LoadShaders()
   {
      m_shadowMapShader.addShaderFromSourceFile(
         QOpenGLShader::Vertex,
         ":/Shaders/shadowMapping.vert");

      m_shadowMapShader.addShaderFromSourceFile(
         QOpenGLShader::Fragment,
         ":/Shaders/shadowMapping.frag");

      auto success = m_shadowMapShader.link();

      success &= Base::LoadShaders("visualizationVertexShader", "visualizationFragmentShader");
      success &= LoadTexturePreviewShaders();

      return success;
   }

   bool Treemap::Initialize()
   {
      const bool unitBlockInitialized = InitializeReferenceBlock();
      const bool transformationsInitialized = InitializeBlockTransformations();
      const bool colorsInitialized = InitializeColors();
      const bool shadowMachineryInitialized = InitializeShadowMachinery();
      const bool texturePreviewerInitialized = InitializeTexturePreviewer();

      const bool overallSuccess =
         unitBlockInitialized
         && transformationsInitialized
         && colorsInitialized
         && shadowMachineryInitialized
         && texturePreviewerInitialized;

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

      m_mainShader.enableAttributeArray("vertex");
      m_mainShader.setAttributeBuffer(
         /* location = */ "vertex",
         /* type = */ GL_FLOAT,
         /* offset = */ 0,
         /* tupleSize = */ 3,
         /* stride = */ 2 * sizeof(QVector3D));

      m_mainShader.enableAttributeArray("normal");
      m_mainShader.setAttributeBuffer(
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

   bool Treemap::InitializeShadowMachinery()
   {
      InitializeShadowMachineryOnMainShader();
      InitializeShadowMachineryOnShadowShader();

      return true;
   }

   void Treemap::InitializeShadowMachineryOnMainShader()
   {
      m_mainShader.bind();

      const auto shaderID = m_mainShader.programId();

      const auto cascadeBounds = ComputeCascadeDistances();
      for (auto index{ 0u }; index < cascadeBounds.size(); ++index)
      {
         const auto indexAsString = std::to_string(index);

         auto variableName = "cascadeBounds[" + indexAsString + "]";
         m_mainShader.setUniformValue(variableName.data(), cascadeBounds[index].second);

         variableName = "shadowMaps[" + indexAsString + "]";
         const auto location = m_openGL.glGetUniformLocation(shaderID, variableName.data());
         m_openGL.glUniform1i(location, index);
      }

      m_mainShader.release();
   }

   void Treemap::InitializeShadowMachineryOnShadowShader()
   {
      m_VAO.bind();
      m_referenceBlockBuffer.bind();
      m_shadowMapShader.bind();

      m_shadowMapShader.enableAttributeArray("vertex");
      m_shadowMapShader.setAttributeBuffer(
         /* location = */ "vertex",
         /* type = */ GL_FLOAT,
         /* offset = */ 0,
         /* tupleSize = */ 3,
         /* stride = */ 2 * sizeof(QVector3D));

      m_shadowMapShader.enableAttributeArray("normal");
      m_shadowMapShader.setAttributeBuffer(
         /* location = */ "normal",
         /* type = */ GL_FLOAT,
         /* offset = */ sizeof(QVector3D),
         /* tupleSize = */ 3,
         /* stride = */ 2 * sizeof(QVector3D));

      m_shadowMapShader.release();
      m_referenceBlockBuffer.release();
      m_VAO.release();
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
         const auto fileIsTooSmall = (node->file.size < parameters.minimumFileSize);
         const auto notTheRightFileType = parameters.onlyShowDirectories
            && node->file.type != FileType::DIRECTORY;

         if (notTheRightFileType || fileIsTooSmall)
         {
            continue;
         }

         node->offsetIntoVBO = m_blockCount++;

         const auto& block = node->block;
         const auto& blockOrigin = block.GetOrigin();

         QMatrix4x4 instanceMatrix;
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
         const auto fileIsTooSmall = (node->file.size < parameters.minimumFileSize);
         const auto notTheRightFileType = parameters.onlyShowDirectories
            && node->file.type != FileType::DIRECTORY;

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
      const auto blockSize = static_cast<long double>(node.GetData().file.size);
      const auto ratio = blockSize / static_cast<long double>(m_largestDirectorySize);

      const auto finalColor = m_directoryColorGradient.GetColorAtValue(static_cast<float>(ratio));
      return finalColor;
   }

   void Treemap::ComputeAppropriateBlockColor(
      const Tree<VizFile>::Node& node,
      const Settings::Manager& settings)
   {
      // @todo Need to also take into consideration whether the node is highlighted or selected,
      // since we don't want to get out of sync with the controller's view of the world.

      if (settings.GetActiveColorScheme() != Constants::ColorScheme::DEFAULT)
      {
         const auto fileColor = DetermineColorFromExtension(node, settings);
         if (fileColor)
         {
            m_blockColors << *fileColor;
            return;
         }
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

   void Treemap::ComputeShadowMapProjectionViewMatrices(const Camera& camera)
   {
      const auto view = ComputeLightViewMatrix();
      const auto cascadeBoundingBoxes = ComputeFrustumSplitBoundingBoxes(camera, view);

      constexpr auto nearPlane{ 10 };
      constexpr auto farPlane{ 1500 };

      for (auto index{ 0u }; index < CASCADE_COUNT; ++index)
      {
         const auto& boundingBox = cascadeBoundingBoxes[index];
         QMatrix4x4 projection;
         projection.ortho(
            boundingBox.left,
            boundingBox.right,
            boundingBox.bottom,
            boundingBox.top,
            nearPlane,
            farPlane);

         const QMatrix4x4 model;
         auto projectionViewMatrix = projection * view * model;
         m_shadowMaps[index].projectionViewMatrix = std::move(projectionViewMatrix);
      }
   }

   void Treemap::RenderShadowPass(const Camera& camera)
   {
      // @note In order to fix Peter-panning artifacts, we'll temporarily cull front faces.
      // This will make the shadow map look rather weird; almost like an outline.
      //m_openGL.glCullFace(GL_FRONT);

      m_openGL.glViewport(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);

      m_shadowMapShader.bind();
      m_VAO.bind();

      for (auto index{ 0u }; index < CASCADE_COUNT; ++index)
      {
         m_shadowMaps[index].framebuffer->bind();

         const auto& projectionViewMatrix = m_shadowMaps[index].projectionViewMatrix;
         m_shadowMapShader.setUniformValue("lightProjectionViewMatrix", projectionViewMatrix);

         m_openGL.glClear(GL_DEPTH_BUFFER_BIT);

         static constexpr float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
         m_openGL.glClearBufferfv(GL_COLOR, 0, white);

         m_openGL.glDrawArraysInstanced(
            /* mode = */ GL_TRIANGLES,
            /* first = */ 0,
            /* count = */ m_referenceBlockVertices.size(),
            /* instanceCount = */ m_blockColors.size()
         );

         m_shadowMaps[index].framebuffer->release();
      }

      m_VAO.release();
      m_shadowMapShader.release();

      const auto& viewport = camera.GetViewport();
      m_openGL.glViewport(0, 0, viewport.width(), viewport.height());

      //m_openGL.glCullFace(GL_BACK);
   }

   void Treemap::RenderMainPass(
      const Camera& camera,
      const std::vector<Light>& lights,
      const Settings::Manager& settings)
   {
      m_openGL.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      m_mainShader.bind();

      const auto shouldShowShadows = settings.ShouldShowShadows();

      m_mainShader.setUniformValue("cameraProjectionViewMatrix", camera.GetProjectionViewMatrix());
      m_mainShader.setUniformValue("cameraPosition", camera.GetPosition());

      // @todo The following variables don't need to be set with every pass...
      m_mainShader.setUniformValue("materialShininess", settings.GetMaterialShininess());
      m_mainShader.setUniformValue("shouldShowCascadeSplits", settings.ShouldShowCascadeSplits());
      m_mainShader.setUniformValue("shouldShowShadows", shouldShowShadows);

      SetUniformLights(lights, settings, m_mainShader);

      if (shouldShowShadows)
      {
         assert(m_shadowMaps.size() == CASCADE_COUNT);

         for (auto index{ 0u }; index < CASCADE_COUNT; ++index)
         {
            const auto matrix = "lightProjectionViewMatrices[" + std::to_string(index) + "]";
            m_mainShader.setUniformValue(matrix.data(), m_shadowMaps[index].projectionViewMatrix);

            m_openGL.glActiveTexture(GL_TEXTURE0 + index);
            m_openGL.glBindTexture(GL_TEXTURE_2D, m_shadowMaps[index].framebuffer->texture());
         }
      }

      m_VAO.bind();

      m_openGL.glDrawArraysInstanced(
         /* mode = */ GL_TRIANGLES,
         /* first = */ 0,
         /* count = */ m_referenceBlockVertices.size(),
         /* instanceCount = */ m_blockColors.size()
      );

      m_VAO.release();

      m_mainShader.release();
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

      if (settings.ShouldShowShadows())
      {
         ComputeShadowMapProjectionViewMatrices(camera);
         RenderShadowPass(camera);
      }

      RenderMainPass(camera, lights, settings);

      //RenderDepthMapPreview(1); //< @note Enable this to render the shadow map to the screen.

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
      assert(node->offsetIntoVBO < m_blockCount);

      constexpr auto colorTupleSize{ sizeof(QVector3D) };
      const auto offsetIntoColorBuffer = node->offsetIntoVBO * colorTupleSize;

      QVector3D newColor;

      switch (action)
      {
         case Asset::Event::SELECT:
         {
            newColor = Constants::Colors::CANARY_YELLOW;
            break;
         }
         case Asset::Event::HIGHLIGHT:
         {
            newColor = Constants::Colors::SLATE_GRAY;
            break;
         }
         default:
         {
            newColor = RestoreColor(node, settings);
         }
      }

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

   bool Treemap::LoadTexturePreviewShaders()
   {
      if (!m_texturePreviewShader.addShaderFromSourceFile(
         QOpenGLShader::Vertex,
         ":/Shaders/texturePreview.vert"))
      {
         std::cout << "Error loading vertex shader!" << std::endl;
      }

      if (!m_texturePreviewShader.addShaderFromSourceFile(
         QOpenGLShader::Fragment,
         ":/Shaders/texturePreview.frag"))
      {
         std::cout << "Error loading fragment shader!" << std::endl;
      }

      return m_texturePreviewShader.link();
   }

   bool Treemap::InitializeTexturePreviewer()
   {
      static constexpr int coordinates[4][3] =
      {
         { +1, -1, -1 },
         { -1, -1, -1 },
         { -1, +1, -1 },
         { +1, +1, -1 }
      };

      m_texturePreviewShader.bindAttributeLocation(
         "vertex",
         TEXTURE_PREVIEWER_VERTEX_ATTRIBUTE);

      m_texturePreviewShader.bindAttributeLocation(
         "texCoord",
         TEXTURE_PREVIEWER_TEXCOORD_ATTRIBUTE);

      QVector<GLfloat> vertexData;
      for (int i = 0; i < 4; ++i)
      {
         // Vertex position:
         vertexData.append(coordinates[i][0]);
         vertexData.append(coordinates[i][1]);
         vertexData.append(coordinates[i][2]);

         // Texture coordinate:
         vertexData.append(i == 0 || i == 3);
         vertexData.append(i == 0 || i == 1);
      }

      m_texturePreviewVertexBuffer.create();
      m_texturePreviewVertexBuffer.bind();

      m_texturePreviewVertexBuffer.allocate(
         vertexData.constData(),
         vertexData.count() * sizeof(GLfloat));

      m_texturePreviewVertexBuffer.release();

      return true;
   }

   void Treemap::RenderDepthMapPreview(int index)
   {
      // Simply using Normalized Device Coordinates (NDC), and an arbitrary choice of view planes.
      QMatrix4x4 viewMatrix;
      viewMatrix.ortho(-1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1000.0f);
      viewMatrix.translate(0.0f, 0.0f, -1.0f);

      m_texturePreviewVertexBuffer.bind();

      m_texturePreviewShader.bind();
      m_texturePreviewShader.setUniformValue("matrix", viewMatrix);
      m_texturePreviewShader.enableAttributeArray(TEXTURE_PREVIEWER_VERTEX_ATTRIBUTE);
      m_texturePreviewShader.enableAttributeArray(TEXTURE_PREVIEWER_TEXCOORD_ATTRIBUTE);

      m_texturePreviewShader.setAttributeBuffer(
         TEXTURE_PREVIEWER_VERTEX_ATTRIBUTE,
         GL_FLOAT,
         /* offset = */    0,
         /* tupleSize = */ 3,
         /* stride = */    5 * sizeof(GLfloat));

      m_texturePreviewShader.setAttributeBuffer(
         TEXTURE_PREVIEWER_TEXCOORD_ATTRIBUTE,
         GL_FLOAT,
         /* offset = */    3 * sizeof(GLfloat),
         /* tupleSize = */ 2,
         /* stride = */    5 * sizeof(GLfloat));

      m_openGL.glActiveTexture(GL_TEXTURE0);
      m_openGL.glBindTexture(GL_TEXTURE_2D, m_shadowMaps[index].framebuffer->texture());

      m_openGL.glDrawArrays(
         /* mode = */ GL_TRIANGLE_FAN,
         /* first = */ 0,
         /* count = */ 4);

      m_texturePreviewShader.release();
      m_texturePreviewVertexBuffer.release();
   }
}
