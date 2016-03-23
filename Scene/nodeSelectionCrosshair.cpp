#include "nodeSelectionCrosshair.h"

#include <iostream>

namespace
{
   /**
    * @brief CreateCrosshairVertices
    * @return
    */
   QVector<QVector3D> CreateCrosshairVertices(const QPoint center)
   {
      QVector<QVector3D> vertices;
      vertices << QVector3D(center.x() - 100, center.y(), -4.0f)
               << QVector3D(center.x() + 100, center.y(), -4.0f)
               << QVector3D(center.x(), center.y() - 100, -4.0f)
               << QVector3D(center.x(), center.y() + 100, -4.0f);

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
   // @bug Returning early for now, since there is something wrong with the rendering code
   // that will cause the entire application to render as a black void. As of yet, I have no
   // clue what's going on. On the plus side, the code below does appear to work for rendering
   // a usable HUD.
   //return true;

   m_graphicsDevice.glDepthMask(GL_FALSE);
   m_graphicsDevice.glDisable(GL_DEPTH_TEST);

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

   m_graphicsDevice.glLineWidth(3);
   m_graphicsDevice.glDrawArrays(
      /* mode = */ GL_LINES,
      /* first = */ 0,
      /* count = */ m_rawVertices.size());

   m_VAO.release();
   m_shader.release();

   m_graphicsDevice.glEnable(GL_DEPTH_TEST);
   m_graphicsDevice.glDepthMask(GL_TRUE);

   return true;
}
