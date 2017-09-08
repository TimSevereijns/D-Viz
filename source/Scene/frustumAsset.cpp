#include "frustumAsset.h"

#include "../constants.h"

namespace
{
   /**
    * @brief The Frustum struct provides an encapsulation of all the data needed to define and
    * consume a single view frustum.
    */
   struct Frustum
   {
      struct Plane
      {
         PrecisePoint center;
         double width;
         double height;

         auto TopLeft() const noexcept
         {
            return QVector3D
            {
               static_cast<float>(center.x() - (width * 0.5)),
               static_cast<float>(center.y() + (height * 0.5)),
               static_cast<float>(center.z())
            };
         }

         auto TopRight() const noexcept
         {
            return QVector3D
            {
               static_cast<float>(center.x() + (width * 0.5)),
               static_cast<float>(center.y() + (height * 0.5)),
               static_cast<float>(center.z())
            };
         }

         auto BottomLeft() const noexcept
         {
            return QVector3D
            {
               static_cast<float>(center.x() - (width * 0.5)),
               static_cast<float>(center.y() - (height * 0.5)),
               static_cast<float>(center.z())
            };
         }

         auto BottomRight() const noexcept
         {
            return QVector3D
            {
               static_cast<float>(center.x() + (width * 0.5)),
               static_cast<float>(center.y() - (height * 0.5)),
               static_cast<float>(center.z())
            };
         }
      };

      auto Depth() const noexcept
      {
         return farPlane.center.z() - nearPlane.center.z();
      }

      Plane nearPlane;
      Plane farPlane;
   };

   /**
    * @brief Generates all of the frustum vertices for the specified camera.
    *
    * @param[in] camera             Main scene camera.
    * @param[in, out] frustumAsset  The main frustum scene asset.
    */
   auto GenerateFrustum(
      const Camera& camera,
      FrustumAsset& frustumAsset)
   {
      std::vector<QVector3D> unitCube
      {
         { -1, -1, -1 }, { +1, -1, -1 },
         { +1, +1, -1 }, { -1, +1, -1 },
         { -1, -1, +1 }, { +1, -1, +1 },
         { +1, +1, +1 }, { -1, +1, +1 }
      };

      const auto invertedProjectionViewMatrix = camera.GetProjectionViewMatrix().inverted();
      for (auto& corner : unitCube)
      {
         corner = invertedProjectionViewMatrix.map(corner);
      }

      QVector<QVector3D> vertices;
      vertices
         // Near plane outline:
         << unitCube[0] << unitCube[1]
         << unitCube[1] << unitCube[2]
         << unitCube[2] << unitCube[3]
         << unitCube[3] << unitCube[0]
         // Far plane outline:
         << unitCube[4] << unitCube[5]
         << unitCube[5] << unitCube[6]
         << unitCube[6] << unitCube[7]
         << unitCube[7] << unitCube[4]
         // Side plane outline:
         << unitCube[0] << unitCube[4]
         << unitCube[1] << unitCube[5]
         << unitCube[2] << unitCube[6]
         << unitCube[3] << unitCube[7];

      const auto cameraPosition = camera.GetPosition();

      Frustum frustum;

      frustum.nearPlane = Frustum::Plane
      {
         PrecisePoint{ cameraPosition.x(), cameraPosition.y(), unitCube[0].z() },
         unitCube[2].x() - unitCube[0].x(),
         unitCube[3].y() - unitCube[0].y()
      };

      frustum.farPlane = Frustum::Plane
      {
         PrecisePoint{ cameraPosition.x(), cameraPosition.y(), unitCube[4].z() },
         unitCube[6].x() - unitCube[4].x(),
         unitCube[7].y() - unitCube[4].y()
      };

      QVector<QVector3D> colors;
      for (auto index{ 0 }; index < vertices.size(); ++index)
      {
         colors << Constants::Colors::CORAL;
      }

      frustumAsset.AddVertexCoordinates(std::move(vertices));
      frustumAsset.AddVertexColors(std::move(colors));

      return frustum;
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
    * @brief GenerateFrustraBoundingBoxes
    *
    * @param[in] frusta             Vector containing all the furstum to be outlined.
    * @param[in, out] frustumAsset  The main frustum scene asset.
    */
   void GenerateFrustaBoundingBoxes(
      const std::vector<Frustum>& frusta,
      FrustumAsset& frustumAsset)
   {
      QVector<QVector3D> vertices;
      for (const auto& frustum : frusta)
      {
         const QVector3D depth{ 0.0f, 0.0f, static_cast<float>(frustum.Depth()) };

         auto nearBottomLeft = frustum.farPlane.BottomLeft();
         nearBottomLeft -= depth;

         auto nearBottomRight = frustum.farPlane.BottomRight();
         nearBottomRight -= depth;

         auto nearTopLeft = frustum.farPlane.TopLeft();
         nearTopLeft -= depth;

         auto nearTopRight = frustum.farPlane.TopRight();
         nearTopRight -= depth;

         vertices
            // Near Plane Outline:
            << nearBottomLeft  << nearBottomRight
            << nearBottomRight << nearTopRight
            << nearTopRight    << nearTopLeft
            << nearTopLeft     << nearBottomLeft
            // Far Plane Outline:
            << frustum.farPlane.BottomLeft()  << frustum.farPlane.BottomRight()
            << frustum.farPlane.BottomRight() << frustum.farPlane.TopRight()
            << frustum.farPlane.TopRight()    << frustum.farPlane.TopLeft()
            << frustum.farPlane.TopLeft()     << frustum.farPlane.BottomLeft()
            // Side Plane Outline:
            << nearBottomLeft  << frustum.farPlane.BottomLeft()
            << nearBottomRight << frustum.farPlane.BottomRight()
            << nearTopRight    << frustum.farPlane.TopRight()
            << nearTopLeft     << frustum.farPlane.TopLeft();
      }

      QVector<QVector3D> colors;
      for (auto index{ 0 }; index < vertices.size(); ++index)
      {
         colors << Constants::Colors::HOT_PINK;
      }

      frustumAsset.AddVertexCoordinates(std::move(vertices));
      frustumAsset.AddVertexColors(std::move(colors));
   }

   /**
    * @brief Helper function to draw the frustum belonging to single stationary scene camera.
    *
    * @param[in, out] frustumAsset  The main frustum scene asset.
    * @param[in] camera             The main camera used to render the scene. Mainly use is to get
    *                               the aspect ratio of the outline correct.
    */
   void GenerateCameraFrusta(
      FrustumAsset& frustumAsset,
      const Camera& camera)
   {
      Camera stationaryCamera = camera;
      stationaryCamera.SetPosition(QVector3D{ 500, 100, 0 });
      stationaryCamera.SetOrientation(0.0f, 0.0f);
      stationaryCamera.SetNearPlane(1.0f);
      stationaryCamera.SetFarPlane(2000.0f);

      constexpr auto cascadeCount{ 3 };
      const auto cascades = GenerateShadowMapCascades(cascadeCount, stationaryCamera);

      std::vector<Frustum> frusta;
      frusta.reserve(cascades.size());

      for (const auto& nearAndFarPlanes : cascades)
      {
         stationaryCamera.SetNearPlane(nearAndFarPlanes.first);
         stationaryCamera.SetFarPlane(nearAndFarPlanes.second);

         auto&& frustum = GenerateFrustum(stationaryCamera, frustumAsset);
         frusta.emplace_back(std::forward<Frustum>(frustum));
      }

      GenerateFrustaBoundingBoxes(frusta, frustumAsset);
   }

   /**
    * @brief Helper function to draw a single shadow caster's perspective.
    *
    * @param[in, out] frustumAsset  The main frustum scene asset.
    * @param[in] camera             The main camera used to render the scene. Mainly use is to get
    *                               the aspect ratio of the outline correct.
    */
   void GenerateShadowCasterFrustum(
      FrustumAsset& frustumAsset,
      const Camera& camera)
   {
      Camera shadowCamera = camera;
      shadowCamera.SetPosition(QVector3D{ -200.0f, 500.0f, 200.0f });
      shadowCamera.SetOrientation(25.0f, 45.0f);
      shadowCamera.SetNearPlane(250.0f);
      shadowCamera.SetFarPlane(800.0f);

      GenerateFrustum(shadowCamera, frustumAsset);
   }
}

FrustumAsset::FrustumAsset(QOpenGLExtraFunctions& renderingContext) :
   LineAsset{ renderingContext }
{
}

bool FrustumAsset::Render(
   const Camera& camera,
   const std::vector<Light>& /*lights*/,
   const OptionsManager& /*settings*/)
{
   if (!m_shouldRender)
   {
      return false;
   }

   m_mainShader.bind();
   m_mainShader.setUniformValue("mvpMatrix", camera.GetProjectionViewMatrix());

   m_VAO.bind();

   m_graphicsDevice.glLineWidth(2);

   m_graphicsDevice.glDrawArrays(
      /* mode = */ GL_LINES,
      /* first = */ 0,
      /* count = */ m_rawVertices.size());

   m_graphicsDevice.glLineWidth(1);

   m_mainShader.release();
   m_VAO.release();

   return true;
}

void FrustumAsset::GenerateFrusta(const Camera& camera)
{
   ClearBuffers();

   GenerateCameraFrusta(*this, camera);
   GenerateShadowCasterFrustum(*this, camera);

   Reload();
}
