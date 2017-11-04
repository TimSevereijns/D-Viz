#include "lightMarkerAsset.h"

namespace Asset
{
   LightMarker::LightMarker(QOpenGLExtraFunctions& openGL) :
      Line{ openGL }
   {
   }

   bool LightMarker::Render(
      const Camera& camera,
      const std::vector<Light>&,
      const OptionsManager&)
   {
      if (!m_shouldRender)
      {
         return false;
      }

      m_shader.bind();
      m_shader.setUniformValue("mvpMatrix", camera.GetProjectionViewMatrix());

      m_VAO.bind();

      m_openGL.glLineWidth(2);

      m_openGL.glDrawArrays(
         /* mode = */ GL_LINES,
         /* first = */ 0,
         /* count = */ m_rawVertices.size());

      m_openGL.glLineWidth(1);

      m_shader.release();
      m_VAO.release();

      return true;
   }
}
