#include "debuggingRayAsset.h"

namespace Asset
{
   DebuggingRay::DebuggingRay(QOpenGLExtraFunctions& openGL) :
      Line{ openGL }
   {
   }

   bool DebuggingRay::Render(
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

      m_openGL.glLineWidth(3);

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
