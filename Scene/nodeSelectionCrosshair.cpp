#include "nodeSelectionCrosshair.h"

namespace
{
   /**
    * @brief CreateCrosshairVertices
    * @return
    */
   QVector<QVector3D> CreateCrosshairVertices()
   {
      QVector<QVector3D> vertices;
      vertices << QVector3D(-100, 0.0f, -4.0f)
               << QVector3D(+100, 0.0f, -4.0f)
               << QVector3D(0.0f, - 100, -4.0f)
               << QVector3D(0.0f, + 100, -4.0f);

      return vertices;
   }

   /**
    * @brief CreateCrosshairColors
    * @return
    */
   QVector<QVector3D> CreateCrosshairColors()
   {
      QVector<QVector3D> colors;
      colors << QVector3D(1.0f, 1.0f, 1.0f)
             << QVector3D(1.0f, 1.0f, 1.0f)
             << QVector3D(1.0f, 1.0f, 1.0f)
             << QVector3D(1.0f, 1.0f, 1.0f);

      return colors;
   }
}

NodeSelectionCrosshair::NodeSelectionCrosshair(GraphicsDevice& device)
   : LineAsset(device)
{
   m_rawColors = CreateCrosshairVertices();
   m_rawColors = CreateCrosshairColors();
}

void NodeSelectionCrosshair::ShowCrosshair()
{
   // @todo Write a HUD shader to just keep these points in front of the camera at all times.
   m_rawVertices = CreateCrosshairVertices();
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
   // @todo Add an early return to "hide" the crosshair, instead of clearing the buffer.
   return true;

   m_shader.bind();
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
