#include "gamepadMenuAsset.h"

#include <cmath>

#include "../Viewport/glCanvas.h"

namespace
{
   constexpr auto Pi = 3.14159265358979323846;

   /**
    * @brief ComputeAttachmentPoints
    *
    * @param origin
    * @param entries
    */
   auto ComputeAttachmentPoints(
      QPoint origin,
      int radius,
      int entryCount)
   {
      std::vector<QPoint> attachmentPoints;
      attachmentPoints.reserve(entryCount);

      const auto slice = 2 * Pi / entryCount;

      const auto x = origin.x();
      const auto y = origin.y();

      for (auto index{ 0 }; index < entryCount; ++index)
      {
         const auto angle = slice * index;

         auto point = QPoint
         {
            static_cast<int>(x + radius * std::cos(angle)),
            static_cast<int>(y + radius * std::sin(angle))
         };

         attachmentPoints.emplace_back(std::move(point));
      }

      return attachmentPoints;
   }
}

namespace Asset
{
   GamepadMenu::GamepadMenu(
      QOpenGLExtraFunctions& openGL,
      bool /* isInitiallyVisible */)
      :
      Line{ openGL, /* isInitiallyVisible = */false }
   {
   }

   bool GamepadMenu::Render(
      const Camera& camera,
      const std::vector<Light>&,
      const Settings::Manager&)
   {
      if (!m_shouldRender)
      {
         return false;
      }

      const auto& viewport = camera.GetViewport();
      QMatrix4x4 orthoMatrix;
      orthoMatrix.ortho(
         viewport.left(),
         viewport.right(),
         viewport.bottom(),
         viewport.top(),
         camera.GetNearPlane(),
         camera.GetFarPlane());

      QMatrix4x4 identityMatrix;
      identityMatrix.setToIdentity();

      const auto mvpMatrix = orthoMatrix * identityMatrix;

      m_shader.bind();
      m_shader.setUniformValue("mvpMatrix", mvpMatrix);

      m_VAO.bind();

      m_openGL.glLineWidth(2);
      m_openGL.glDrawArrays(
         /* mode = */ GL_LINES,
         /* first = */ 0,
         /* count = */ m_rawVertices.size());

      m_VAO.release();
      m_shader.release();

      RenderText(camera);

      return true;
   }

   void GamepadMenu::RenderText(const Camera& camera)
   {
      m_painter.begin(m_context);
      m_painter.setPen(Qt::green);
      m_painter.setFont(m_font);
      m_painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
      m_painter.drawText(camera.GetViewport(), Qt::AlignCenter, "Qt");
      m_painter.end();
   }

   void GamepadMenu::Construct(
      QPoint origin,
      const std::vector<std::pair<QString, std::function<void ()>>>& entries)
   {
      constexpr auto radius{ 100 };
      constexpr auto segmentCount{ 64 };

      AddCircle(origin, radius, segmentCount);

      const auto attachmentPoints = ComputeAttachmentPoints(origin, radius, entries.size());
   }

   void GamepadMenu::SetRenderContext(GLCanvas* context)
   {
      m_context = context;
   }

   void GamepadMenu::AddCircle(
      QPoint origin,
      int radius,
      int segmentCount)
   {
      const auto slice = 2 * Pi / segmentCount;

      const auto x = origin.x();
      const auto y = origin.y();

      m_rawVertices.reserve(segmentCount + 1);
      m_rawColors.reserve(segmentCount + 1);

      for (auto index{ 0 }; index < segmentCount; ++index)
      {
         const auto angle = slice * index;

         auto vertex = QVector3D
         {
            static_cast<float>(x + radius * std::cos(angle)),
            static_cast<float>(y + radius * std::sin(angle)),
            -4.0f //< @note: Somewhat arbitrarily chosen...
         };

         m_rawVertices << std::move(vertex);
         m_rawColors << Constants::Colors::WHITE;
      }

      // Close the loop:
      m_rawVertices << m_rawVertices[0];
      m_rawColors << m_rawColors[0];

      Refresh();
   }
}
