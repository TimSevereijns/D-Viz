#include "lightMarkerAsset.h"

namespace Asset
{
   LightMarker::LightMarker(
      QOpenGLExtraFunctions& openGL,
      bool isInitiallyVisible)
      :
      Line{ openGL, isInitiallyVisible }
   {
   }

   bool LightMarker::Render(
      const Camera& camera,
      const std::vector<Light>&,
      const Settings::Manager&)
   {
      if (!m_shouldRender)
      {
         return false;
      }

      m_mainShader.bind();
      m_mainShader.setUniformValue("mvpMatrix", camera.GetProjectionViewMatrix());

      m_VAO.bind();

      m_openGL.glLineWidth(2);

      m_openGL.glDrawArrays(
         /* mode = */ GL_LINES,
         /* first = */ 0,
         /* count = */ m_rawVertices.size());

      m_openGL.glLineWidth(1);

      m_mainShader.release();
      m_VAO.release();

      return true;
   }
}
