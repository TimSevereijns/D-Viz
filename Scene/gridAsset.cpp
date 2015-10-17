#include "gridAsset.h"

#include "../Visualizations/visualization.h"

namespace
{
   /**
    * @brief CreateOriginMarkerVertices returns the vertices needed to render the coordinate
    * system origin marker.
    *
    * @returns a vector of vertices.
    */
   QVector<QVector3D> CreateOriginMarkerVertices()
   {
      const float markerAxisLength = Visualization::ROOT_BLOCK_WIDTH;

      QVector<QVector3D> marker;
      marker
         << QVector3D(0.0f, 0.0f, 0.0f) << QVector3D(markerAxisLength, 0.0f, 0.0f)   // X-axis
         << QVector3D(0.0f, 0.0f, 0.0f) << QVector3D(0.0f, 100.0f, 0.0f)             // Y-axis
         << QVector3D(0.0f, 0.0f, 0.0f) << QVector3D(0.0f, 0.0f, -markerAxisLength)  // Z-axis

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

      return marker;
   }

   /**
    * @brief CreateOriginMarkerColors returns the vertex colors needed to paint the origin marker.
    *
    * @returns a vector of vertex colors.
    */
   QVector<QVector3D> CreateOriginMarkerColors()
   {
      QVector<QVector3D> markerColors;
      markerColors
         << QVector3D(1.0f, 1.0f, 1.0f) << QVector3D(1.0f, 0.0f, 0.0f)  // X-axis (red)
         << QVector3D(1.0f, 1.0f, 1.0f) << QVector3D(0.0f, 1.0f, 0.0f)  // Y-axis (green)
         << QVector3D(1.0f, 1.0f, 1.0f) << QVector3D(0.0f, 0.0f, 1.0f)  // Z-axis (blue)

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

      return markerColors;
   }
}

GridAsset::GridAsset(GraphicsDevice& device)
   : SceneAsset(device)
{
   m_rawVertices = CreateOriginMarkerVertices();
   m_rawColors = CreateOriginMarkerColors();
}

bool GridAsset::LoadShaders()
{
   return SceneAsset::LoadShaders("originMarkerVertexShader", "originMarkerFragmentShader");
}

bool GridAsset::PrepareVertexBuffers(const Camera& camera)
{
   m_VAO.create(); ///< This is a bit a hack: I happen to know that I load vertices before colors
   m_VAO.bind();

   m_vertexBuffer.create();
   m_vertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
   m_vertexBuffer.bind();
   m_vertexBuffer.allocate(m_rawVertices.constData(), m_rawVertices.size() * 3 * sizeof(GLfloat));

   m_shader.bind();
   m_shader.setUniformValue("mvpMatrix", camera.GetProjectionViewMatrix());

   m_vertexBuffer.bind();
   m_shader.enableAttributeArray("vertex");
   m_shader.setAttributeBuffer("vertex", GL_FLOAT, /* offset = */ 0,
      /* tupleSize = */ 3, /* stride = */ 6 * sizeof(GLfloat));

   m_vertexBuffer.release();
   m_shader.release();
   m_VAO.release();

   return true;
}

bool GridAsset::PrepareColorBuffers(const Camera& camera)
{
   m_VAO.bind();

   m_colorBuffer.create();
   m_colorBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
   m_colorBuffer.bind();
   m_colorBuffer.allocate(m_rawColors.constData(), m_rawColors.size() * 3 * sizeof(GLfloat));

   m_shader.bind();
   m_shader.setUniformValue("mvpMatrix", camera.GetProjectionViewMatrix());

   m_colorBuffer.bind();
   m_shader.enableAttributeArray("color");
   m_shader.setAttributeBuffer("color", GL_FLOAT, /* offset = */ 0, /* tupleSize = */ 3);

   m_colorBuffer.release();
   m_shader.release();
   m_VAO.release();

   return true;
}

bool GridAsset::Render(const Camera& camera, const Light& light, bool isVizualizationLoaded,
                       const OptionsManager& settings)
{
   m_shader.bind();
   m_shader.setUniformValue("mvpMatrix", camera.GetProjectionViewMatrix());

   m_VAO.bind();

   m_graphicsDevice.glDrawArrays(GL_LINES, /* first = */ 0, /* count = */ m_rawVertices.size());

   m_shader.release();
   m_VAO.release();

   return true;
}

bool GridAsset::Reload(const Camera& camera)
{
   // No-op

   return true;
}
