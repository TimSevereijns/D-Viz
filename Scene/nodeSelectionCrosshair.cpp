#include "nodeSelectionCrosshair.h"

#include <iostream>

namespace
{
   /**
    * @brief Generates the vertices needed to represent the crosshair.
    *
    * @param[in] center             The location at which to center the crosshair.
    *
    * @returns All the vertices needed to draw the crosshair.
    */
   auto CreateCrosshairVertices(const QPoint center)
   {
      QVector<QVector3D> vertices;
      vertices
         << QVector3D(center.x() - 20, center.y(), -4.0f)
         << QVector3D(center.x() + 20, center.y(), -4.0f)
         << QVector3D(center.x(), center.y() - 20, -4.0f)
         << QVector3D(center.x(), center.y() + 20, -4.0f);

      return vertices;
   }

   /**
    * @brief Generates the color data needed to color the crosshair.
    *
    * @returns The color of each vertex needed to represent the crosshair.
    * @see CreateCrosshairVertices
    */
   auto CreateCrosshairColors()
   {
      QVector<QVector3D> colors;
      colors
         << QVector3D(1.0f, 1.0f, 1.0f)
         << QVector3D(1.0f, 1.0f, 1.0f)
         << QVector3D(1.0f, 1.0f, 1.0f)
         << QVector3D(1.0f, 1.0f, 1.0f);

      return colors;
   }
}

NodeSelectionCrosshair::NodeSelectionCrosshair(GraphicsDevice& device) :
   LineAsset{ device }
{
   m_rawColors = CreateCrosshairColors();
}

void NodeSelectionCrosshair::ShowCrosshair(const Camera& camera)
{
   m_rawVertices = CreateCrosshairVertices(camera.GetViewport().center());
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
   const auto& viewPort = camera.GetViewport();
   QMatrix4x4 orthoMatrix;
   orthoMatrix.ortho(
      viewPort.left(),
      viewPort.right(),
      viewPort.bottom(),
      viewPort.top(),
      camera.GetNearPlane(),
      camera.GetFarPlane());

   QMatrix4x4 identityMatrix;
   identityMatrix.setToIdentity();

   const auto mvpMatrix = orthoMatrix * identityMatrix;

   m_shader.bind();
   m_shader.setUniformValue("mvpMatrix", mvpMatrix);

   m_VAO.bind();

   m_graphicsDevice.glLineWidth(2);
   m_graphicsDevice.glDrawArrays(
      /* mode = */ GL_LINES,
      /* first = */ 0,
      /* count = */ m_rawVertices.size());

   m_VAO.release();
   m_shader.release();

   return true;
}
