#include "nodeSelectionCrosshair.h"

namespace
{
   /**
    * @brief CreateCrosshairVertices
    * @return
    */
   QVector<QVector3D> CreateCrosshairVertices(QPoint center)
   {
      QVector<QVector3D> vertices;
      vertices << QVector3D(center.x() - 10, 0.0f, -4.0f)
               << QVector3D(center.x() + 10, 0.0f, -4.0f)
               << QVector3D(0.0f, center.y() - 10, -4.0f)
               << QVector3D(0.0f, center.y() + 10, -4.0f);

      return vertices;
   }

   /**
    * @brief CreateCrosshairColors
    * @return
    */
   QVector<QVector3D> CreateCrosshairColors()
   {
      QVector<QVector3D> colors;
      colors << QVector3D(0.0f, 0.0f, 0.0f)
             << QVector3D(0.0f, 0.0f, 0.0f)
             << QVector3D(0.0f, 0.0f, 0.0f)
             << QVector3D(0.0f, 0.0f, 0.0f);

      return colors;
   }
}

NodeSelectionCrosshair::NodeSelectionCrosshair(GraphicsDevice& device)
   : LineAsset(device)
{
   m_rawColors = CreateCrosshairVertices(QPoint{ 400, 300 });
   m_rawColors = CreateCrosshairColors();
}

void NodeSelectionCrosshair::ShowCrosshairPosition(const QPoint& crosshairCenter)
{
   m_rawVertices = CreateCrosshairVertices(crosshairCenter);
}

void NodeSelectionCrosshair::HideCrosshair()
{
   m_rawVertices.clear();
}

bool NodeSelectionCrosshair::Render(
   const Camera& camera,
   const std::vector<Light>&,
   const OptionsManager&)
{
   m_shader.bind();
   m_shader.setUniformValue("mvpMatrix", camera.GetProjectionViewMatrix());

   m_VAO.bind();

   m_graphicsDevice.glLineWidth(3);
   m_graphicsDevice.glDrawArrays(
      GL_LINES,
      /* first = */ 0,
      /* count = */ m_rawVertices.size());

   m_shader.release();
   m_VAO.release();

   return true;
}
