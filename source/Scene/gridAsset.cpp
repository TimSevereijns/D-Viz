#include "gridAsset.h"

namespace
{
   /**
    * @brief Creates the vertices needed to render the grid.
    *
    * @returns A vector of vertices.
    */
   auto CreateGridVertices()
   {
      QVector<QVector3D> vertices;
      vertices.reserve(44);
      vertices
         // Grid (Z-axis):
         << QVector3D( 0.0f,   0.0f,  0.0f) << QVector3D(   0.0f, 0.0f, -1000.0f)
         << QVector3D( 100.0f, 0.0f,  0.0f) << QVector3D( 100.0f, 0.0f, -1000.0f)
         << QVector3D( 200.0f, 0.0f,  0.0f) << QVector3D( 200.0f, 0.0f, -1000.0f)
         << QVector3D( 300.0f, 0.0f,  0.0f) << QVector3D( 300.0f, 0.0f, -1000.0f)
         << QVector3D( 400.0f, 0.0f,  0.0f) << QVector3D( 400.0f, 0.0f, -1000.0f)
         << QVector3D( 500.0f, 0.0f,  0.0f) << QVector3D( 500.0f, 0.0f, -1000.0f)
         << QVector3D( 600.0f, 0.0f,  0.0f) << QVector3D( 600.0f, 0.0f, -1000.0f)
         << QVector3D( 700.0f, 0.0f,  0.0f) << QVector3D( 700.0f, 0.0f, -1000.0f)
         << QVector3D( 800.0f, 0.0f,  0.0f) << QVector3D( 800.0f, 0.0f, -1000.0f)
         << QVector3D( 900.0f, 0.0f,  0.0f) << QVector3D( 900.0f, 0.0f, -1000.0f)
         << QVector3D(1000.0f, 0.0f,  0.0f) << QVector3D(1000.0f, 0.0f, -1000.0f)

         // Grid (X-axis):
         << QVector3D(0.0f, 0.0f,     0.0f) << QVector3D(1000.0f, 0.0f,     0.0f)
         << QVector3D(0.0f, 0.0f,  -100.0f) << QVector3D(1000.0f, 0.0f,  -100.0f)
         << QVector3D(0.0f, 0.0f,  -200.0f) << QVector3D(1000.0f, 0.0f,  -200.0f)
         << QVector3D(0.0f, 0.0f,  -300.0f) << QVector3D(1000.0f, 0.0f,  -300.0f)
         << QVector3D(0.0f, 0.0f,  -400.0f) << QVector3D(1000.0f, 0.0f,  -400.0f)
         << QVector3D(0.0f, 0.0f,  -500.0f) << QVector3D(1000.0f, 0.0f,  -500.0f)
         << QVector3D(0.0f, 0.0f,  -600.0f) << QVector3D(1000.0f, 0.0f,  -600.0f)
         << QVector3D(0.0f, 0.0f,  -700.0f) << QVector3D(1000.0f, 0.0f,  -700.0f)
         << QVector3D(0.0f, 0.0f,  -800.0f) << QVector3D(1000.0f, 0.0f,  -800.0f)
         << QVector3D(0.0f, 0.0f,  -900.0f) << QVector3D(1000.0f, 0.0f,  -900.0f)
         << QVector3D(0.0f, 0.0f, -1000.0f) << QVector3D(1000.0f, 0.0f, -1000.0f);

      return vertices;
   }

   /**
    * @brief Creates the vertex colors needed to paint the grid.
    *
    * @returns A vector of vertex colors.
    */
   auto CreateGridColors()
   {
      QVector<QVector3D> colors;
      colors.reserve(44);
      colors
         // Grid (Z-axis):
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)

         // Grid (X-axis):
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f)
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f);

      return colors;
   }
}

GridAsset::GridAsset(QOpenGLExtraFunctions& device) :
   LineAsset{ device }
{
   m_rawVertices = CreateGridVertices();
   m_rawColors = CreateGridColors();

   m_shouldRender = false;
}

bool GridAsset::Render(
   const Camera& camera,
   const std::vector<Light>&,
   const OptionsManager&)
{
   if (!m_shouldRender)
   {
      return false;
   }

   m_mainShader.bind();
   m_mainShader.setUniformValue("mvpMatrix", camera.GetProjectionViewMatrix());

   m_VAO.bind();

   m_graphicsDevice.glLineWidth(1);
   m_graphicsDevice.glDrawArrays(
      /* mode = */ GL_LINES,
      /* first = */ 0,
      /* count = */ m_rawVertices.size());

   m_mainShader.release();
   m_VAO.release();

   return true;
}

