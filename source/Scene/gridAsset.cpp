#include "gridAsset.h"

#include "../Visualizations/visualization.h"

namespace
{
   /**
    * @brief Creates the vertices needed to render the coordinate system origin marker.
    *
    * @returns A vector of vertices.
    */
   auto CreateOriginMarkerAndGridVertices()
   {
      const float axisLength = Visualization::ROOT_BLOCK_WIDTH;

      QVector<QVector3D> vertices;
      vertices.reserve(46);
      vertices
         << QVector3D(0.0f, 0.0f, 0.0f) << QVector3D(axisLength, 0.0f, 0.0f)   // X-axis
         << QVector3D(0.0f, 0.0f, 0.0f) << QVector3D(0.0f, 100.0f, 0.0f)       // Y-axis
         << QVector3D(0.0f, 0.0f, 0.0f) << QVector3D(0.0f, 0.0f, -axisLength)  // Z-axis

         // Grid (Z-axis):
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
    * @brief Creates the vertex colors needed to paint the origin marker.
    *
    * @returns A vector of vertex colors.
    */
   auto CreateOriginMarkerAndGridColors()
   {
      QVector<QVector3D> colors;
      colors.reserve(46);
      colors
         << QVector3D(1.0f, 0.0f, 0.0f) << QVector3D(1.0f, 0.0f, 0.0f)  // X-axis (red)
         << QVector3D(0.0f, 1.0f, 0.0f) << QVector3D(0.0f, 1.0f, 0.0f)  // Y-axis (green)
         << QVector3D(0.0f, 0.0f, 1.0f) << QVector3D(0.0f, 0.0f, 1.0f)  // Z-axis (blue)

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
         << QVector3D(1.0f, 1.0f, 0.0f) << QVector3D(1.0f, 1.0f, 0.0f);

      return colors;
   }
}

GridAsset::GridAsset(GraphicsDevice& device) :
   LineAsset{ device }
{
   m_rawVertices = CreateOriginMarkerAndGridVertices();
   m_rawColors = CreateOriginMarkerAndGridColors();
}

bool GridAsset::Render(
   const Camera& camera,
   const std::vector<Light>&,
   const OptionsManager&)
{
   m_shader.bind();
   m_shader.setUniformValue("mvpMatrix", camera.GetProjectionViewMatrix());

   m_VAO.bind();

   m_graphicsDevice.glLineWidth(2);
   m_graphicsDevice.glDrawArrays(
      /* mode = */ GL_LINES,
      /* first = */ 0,
      /* count = */ 6);

   m_graphicsDevice.glLineWidth(1);
   m_graphicsDevice.glDrawArrays(
      /* mode = */ GL_LINES,
      /* first = */ 6,
      /* count = */ m_rawVertices.size());

   m_shader.release();
   m_VAO.release();

   return true;
}

bool GridAsset::Reload()
{
   return true;
}
