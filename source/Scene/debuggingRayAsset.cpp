#include "debuggingRayAsset.h"

DebuggingRayAsset::DebuggingRayAsset(GraphicsDevice& device) :
   LineAsset{ device }
{
}

bool DebuggingRayAsset::Render(
   const Camera& camera,
   const std::vector<Light>&,
   const OptionsManager&)
{
   m_mainShader.bind();
   m_mainShader.setUniformValue("mvpMatrix", camera.GetProjectionViewMatrix());

   m_VAO.bind();

   m_graphicsDevice.glLineWidth(3);

   m_graphicsDevice.glDrawArrays(
      /* mode = */ GL_LINES,
      /* first = */ 0,
      /* count = */ m_rawVertices.size());

   m_graphicsDevice.glLineWidth(1);

   m_mainShader.release();
   m_VAO.release();

   return true;
}
