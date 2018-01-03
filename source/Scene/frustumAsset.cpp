#include "frustumAsset.h"

#include "../constants.h"

#include <algorithm>
#include <iostream>

#include <cmath>

namespace
{
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
    * @brief Generates a frustum outline.
    *
    * @param[in] camera             Main scene camera.
    * @param[in, out] frustumAsset  The main frustum scene asset.
    */
   auto GenerateFrustum(const Camera& camera)
   {
      const auto frustum = ComputeFrustumCorners(camera);

      std::vector<QVector3D> vertices
      {
         // Near plane outline:
         frustum[0], frustum[1],
         frustum[1], frustum[2],
         frustum[2], frustum[3],
         frustum[3], frustum[0],
         // Far plane outline:
         frustum[4], frustum[5],
         frustum[5], frustum[6],
         frustum[6], frustum[7],
         frustum[7], frustum[4],
         // Side plane outline:
         frustum[0], frustum[4],
         frustum[1], frustum[5],
         frustum[2], frustum[6],
         frustum[3], frustum[7]
      };

      return vertices;
   }

   /**
    * @brief Computes the ideal split locations for each frustum cascade.
    *
    * @param[in] cascadeCount       The desired number of shadow mapping cascades.
    * @param[in] camera             The main scene camera.
    */
   auto GenerateShadowMapCascades(
      int cascadeCount,
      const Camera& camera)
   {
      const double nearPlane = camera.GetNearPlane();
      const double farPlane = camera.GetFarPlane();
      const double planeRatio = farPlane / nearPlane;

      auto previousCascadeStart{ nearPlane };

      std::vector<std::pair<double, double>> cascadeDistances;
      for (auto index{ 1.0 }; index < cascadeCount; ++index)
      {
         const auto cascade = nearPlane * std::pow(planeRatio, index / cascadeCount);
         cascadeDistances.emplace_back(std::make_pair(previousCascadeStart, cascade));
         previousCascadeStart = cascade;
      }

      cascadeDistances.emplace_back(std::make_pair(previousCascadeStart, farPlane));

      return cascadeDistances;
   }

   /**
    * @brief Helper function to draw the frustum belonging to single stationary scene camera.
    *
    * @param[in, out] frustumAsset  The main frustum scene asset.
    * @param[in] camera             The main camera used to render the scene. Mainly use is to get
    *                               the aspect ratio of the outline correct.
    */
   void GenerateCameraFrusta(
      Asset::Frustum& frustumAsset,
      const Camera& camera)
   {
      Camera mutableCamera = camera;
      mutableCamera.SetNearPlane(1.0f);
      mutableCamera.SetFarPlane(2000.0f);

      constexpr auto cascadeCount{ 3 };
      const auto cascades = GenerateShadowMapCascades(cascadeCount, mutableCamera);

      std::vector<std::vector<QVector3D>> frusta;
      frusta.reserve(cascadeCount);

      for (const auto& nearAndFarPlanes : cascades)
      {
         mutableCamera.SetNearPlane(nearAndFarPlanes.first);
         mutableCamera.SetFarPlane(nearAndFarPlanes.second);

         frusta.emplace_back(GenerateFrustum(mutableCamera));
      }

      for (auto& frustum : frusta)
      {
         auto vertices = QVector<QVector3D>::fromStdVector(frustum);

         QVector<QVector3D> colors;
         colors.reserve(vertices.size());

         for (auto index{ 0 }; index < vertices.size(); ++index)
         {
            colors << Constants::Colors::HOT_PINK;
         }

         frustumAsset.AddVertexCoordinates(std::move(vertices));
         frustumAsset.AddVertexColors(std::move(colors));
      }
   }

   /**
    * @brief Helper function to draw a single shadow caster's perspective.
    *
    * @param[in, out] frustumAsset  The main frustum scene asset.
    * @param[in] camera             The main camera used to render the scene. Mainly use is to get
    *                               the aspect ratio of the outline correct.
    */
   void GenerateShadowCasterFrustum(
      Asset::Frustum& frustumAsset,
      const Camera& camera)
   {
      const auto& frustum = GenerateFrustum(camera);
      auto vertices = QVector<QVector3D>::fromStdVector(frustum);

      QVector<QVector3D> colors;
      colors.reserve(vertices.size());

      for (auto index{ 0 }; index < vertices.size(); ++index)
      {

         colors << Constants::Colors::CORAL;
      }

      frustumAsset.AddVertexCoordinates(std::move(vertices));
      frustumAsset.AddVertexColors(std::move(colors));
   }

   /**
    * @brief Calculates and sets the vertices needed to visualize the Axis Aligned Bounding Boxes
    * for each of the frustum splits.
    *
    * @param[in, out] frustumAsset  The main frustum scene asset.
    * @param[in] renderCamera       The main camera used to render the scene. Mainly use is to get
    *                               the aspect ratio of the outline correct.
    * @param[in] shadowCamera       The camera that is to render the scene from the light caster's
    *                               perspective.
    */
   void RenderCascadeBoundingBoxes(
      Asset::Frustum& frustumAsset,
      const Camera& renderCamera,
      const Camera& shadowCamera)
   {
      constexpr auto cascadeCount{ 3 };
      const auto cascades = GenerateShadowMapCascades(cascadeCount, renderCamera);

      std::vector<std::vector<QVector3D>> frusta;
      frusta.reserve(cascadeCount);

      auto mutableCamera = renderCamera;
      for (const auto& nearAndFarPlanes : cascades)
      {
         mutableCamera.SetNearPlane(nearAndFarPlanes.first);
         mutableCamera.SetFarPlane(nearAndFarPlanes.second);

         frusta.emplace_back(ComputeFrustumCorners(mutableCamera));
      }

      const auto worldToLight = shadowCamera.GetProjectionViewMatrix();

      QVector<QVector3D> vertices;
      vertices.reserve(24 * cascadeCount);

      for (auto& frustum : frusta)
      {
         auto minX = std::numeric_limits<float>::max();
         auto maxX = std::numeric_limits<float>::lowest();

         auto minY = std::numeric_limits<float>::max();
         auto maxY = std::numeric_limits<float>::lowest();

         auto minZ = std::numeric_limits<float>::max();
         auto maxZ = std::numeric_limits<float>::lowest();

         // Compute bounding box in light space:
         for (auto& vertex : frustum)
         {
            const auto mappedVertex = worldToLight.map(vertex);

            minX = std::min(minX, mappedVertex.x());
            maxX = std::max(maxX, mappedVertex.x());

            minY = std::min(minY, mappedVertex.y());
            maxY = std::max(maxY, mappedVertex.y());

            minZ = std::min(minZ, mappedVertex.z());
            maxZ = std::max(maxZ, mappedVertex.z());
         }

         vertices
            // Near plane:
            << QVector3D{ minX, maxY, maxZ } << QVector3D{ maxX, maxY, maxZ }
            << QVector3D{ maxX, maxY, maxZ } << QVector3D{ maxX, minY, maxZ }
            << QVector3D{ maxX, minY, maxZ } << QVector3D{ minX, minY, maxZ }
            << QVector3D{ minX, minY, maxZ } << QVector3D{ minX, maxY, maxZ }
            // Far plane:
            << QVector3D{ minX, maxY, minZ } << QVector3D{ maxX, maxY, minZ }
            << QVector3D{ maxX, maxY, minZ } << QVector3D{ maxX, minY, minZ }
            << QVector3D{ maxX, minY, minZ } << QVector3D{ minX, minY, minZ }
            << QVector3D{ minX, minY, minZ } << QVector3D{ minX, maxY, minZ }
            // Connect the planes:
            << QVector3D{ minX, maxY, minZ } << QVector3D{ minX, maxY, maxZ }
            << QVector3D{ maxX, maxY, minZ } << QVector3D{ maxX, maxY, maxZ }
            << QVector3D{ maxX, minY, minZ } << QVector3D{ maxX, minY, maxZ }
            << QVector3D{ minX, minY, minZ } << QVector3D{ minX, minY, maxZ };
      }

      const auto lightToWorld = worldToLight.inverted();

      // Transform points back to camera space for visualization:
      for (auto index{ 0 }; index < vertices.size(); ++index)
      {
         vertices[index] = lightToWorld.map(vertices[index]);
      }

      QVector<QVector3D> colors;
      colors.reserve(24 * cascadeCount);

      for (auto index{ 0 }; index < vertices.size(); ++index)
      {
         colors << Constants::Colors::GREEN;
      }

      frustumAsset.AddVertexCoordinates(std::move(vertices));
      frustumAsset.AddVertexColors(std::move(colors));
   }
}

namespace Asset
{
   Frustum::Frustum(
      QOpenGLExtraFunctions& openGL,
      bool isInitiallyVisible)
      :
      Line{ openGL, isInitiallyVisible }
   {
   }

   bool Frustum::Render(
      const Camera& camera,
      const std::vector<Light>& /*lights*/,
      const Settings::Manager& /*settings*/)
   {
      if (!m_shouldRender)
      {
         return false;
      }

      m_mainShader.bind();
      m_mainShader.setUniformValue("mvpMatrix", camera.GetProjectionViewMatrix());

      m_VAO.bind();

      m_openGL.glLineWidth(2);

      m_openGL.glDrawArrays(
         /* mode = */ GL_LINES,
         /* first = */ 0,
         /* count = */ m_rawVertices.size());

      m_openGL.glLineWidth(1);

      m_mainShader.release();
      m_VAO.release();

      return true;
   }

   void Frustum::GenerateFrusta(const Camera& camera)
   {
      ClearBuffers();

      Camera renderCamera = camera;
      renderCamera.SetPosition(QVector3D{ 500, 100, 0 });
      renderCamera.SetOrientation(0.0f, 0.0f);
      renderCamera.SetNearPlane(1.0f);
      renderCamera.SetFarPlane(2000.0f);

      Camera shadowCamera = camera;
      shadowCamera.SetPosition(QVector3D{ -200.0f, 500.0f, 200.0f });
      shadowCamera.SetOrientation(25.0f, 45.0f);
      shadowCamera.SetNearPlane(250.0f);

      RenderCascadeBoundingBoxes(*this, renderCamera, shadowCamera);

      GenerateCameraFrusta(*this, renderCamera);
      GenerateShadowCasterFrustum(*this, shadowCamera);

      Refresh();
   }
}
