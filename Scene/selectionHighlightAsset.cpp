#include "selectionHighlightAsset.h"

SelectionHighlightAsset::SelectionHighlightAsset(GraphicsDevice& device)
   : LineAsset(device)
{
}

bool SelectionHighlightAsset::Render(
   const Camera& camera,
   const std::vector<Light>&,
   const OptionsManager&)
{
   m_shader.bind();
   m_shader.setUniformValue("mvpMatrix", camera.GetProjectionViewMatrix());

   m_VAO.bind();

   m_graphicsDevice.glLineWidth(4);
   m_graphicsDevice.glDrawArrays(GL_LINES, /* first = */ 0, /* count = */ m_rawVertices.size());
   m_graphicsDevice.glLineWidth(1);

   m_shader.release();
   m_VAO.release();

   return true;
}
