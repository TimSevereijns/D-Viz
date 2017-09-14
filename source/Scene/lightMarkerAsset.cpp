#include "lightMarkerAsset.h"

LightMarkerAsset::LightMarkerAsset(QOpenGLExtraFunctions& device) :
   LineAsset{ device }
{
   m_shouldRender = false;
}

bool LightMarkerAsset::Render(
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
