#include "gamepadMenuAsset.h"

#include <cmath>

#include "../constants.h"
#include "../Viewport/glCanvas.h"

namespace
{
   /**
    * @brief Computes the coordinates at which we want to attach menu entries.
    *
    * @param[in] origin             The origin, or center, of the circular menu.
    * @param[in] radius             The radius of the circle that forms the menu.
    * @param[in, out] entries       The menu entries.
    */
   void ComputeAttachmentPoints(
      QPoint origin,
      int radius,
      std::vector<Asset::GamepadMenu::Entry>& entries)
   {
      const auto entryCount = static_cast<int>(entries.size());
      const auto slice = 2 * Constants::Math::Pi / entryCount;
      const auto startingAngle = (entryCount % 2) ? (Constants::Math::Pi / 2) : 0;

      const auto x = origin.x();
      const auto y = origin.y();

      for (auto index{ 0 }; index < entryCount; ++index)
      {
         const auto angle = slice * index - startingAngle;

         entries[index].Position = QPoint
         {
            static_cast<int>(x + radius * std::cos(angle)),
            static_cast<int>(y + radius * std::sin(angle))
         };
      }
   }

   /**
    * @brief Applies a rather simple heuristic to adjust the origin of the menu label so that the
    * label text doesn't interfere with the circle that defines the menu's basic shape.
    *
    * @param[in] entry              A given menu entry.
    * @param[in] menuCenter         The center of the circular menu.
    *
    * @returns The adjusted 2D origin for the menu entry.
    */
   auto AdjustTextOriginBasedOnLocation(
      const Asset::GamepadMenu::Entry entry,
      const QPoint& menuCenter)
   {
      auto adjustedPosition{ entry.Position };

      if (entry.Position.x() == menuCenter.x())
      {
         constexpr auto halfCharacterWide{ 6 };
         adjustedPosition -= QPoint{ entry.Label.size() * halfCharacterWide, 0 };
      }
      else if (entry.Position.x() < menuCenter.x())
      {
         constexpr auto presumedCharacterWide{ 12 };
         adjustedPosition -= QPoint{ entry.Label.size() * presumedCharacterWide, 0 };
      }

      if (entry.Position.y() == menuCenter.y())
      {
         constexpr auto halfCharacterHeight{ 6 };
         adjustedPosition += QPoint{ 0, halfCharacterHeight };
      }
      else if (entry.Position.y() > menuCenter.y())
      {
         constexpr auto presumedCharacterHeight{ 12 };
         adjustedPosition += QPoint{ 0, presumedCharacterHeight };
      }

      return adjustedPosition;
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
      m_font.setFamily("Courier"); // @todo Look into using the QFontDatabase instead.
      m_font.setPointSize(16);
      m_font.setBold(true);
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

      RenderLabels(camera);

      return true;
   }

   void GamepadMenu::RenderLabels(const Camera& camera)
   {
      m_painter.begin(m_context);

      m_painter.setPen(Qt::green);
      m_painter.setFont(m_font);
      m_painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
      m_painter.drawText(camera.GetViewport(), Qt::AlignCenter, "D-Viz");

      for (const auto& entry : m_entries)
      {
         const auto adjustedOrigin = AdjustTextOriginBasedOnLocation(entry, m_menuOrigin);
         m_painter.drawText(adjustedOrigin, entry.Label);
      }

      m_painter.end();
   }

   void GamepadMenu::Construct(
      const QPoint& origin,
      const std::vector<Entry>& entries)
   {
      m_menuOrigin = origin;
      m_entries = std::move(entries);

      constexpr auto radius{ 100 };
      constexpr auto segmentCount{ 64 };

      AddCircle(origin, radius, segmentCount);

      ComputeAttachmentPoints(origin, 1.25 * radius, m_entries);
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
      const auto slice = 2 * Constants::Math::Pi / segmentCount;
      const auto startingAngle = (segmentCount % 2) ? (Constants::Math::Pi / 2) : 0;

      const auto x = origin.x();
      const auto y = origin.y();

      m_rawVertices.reserve(segmentCount + 1);
      m_rawColors.reserve(segmentCount + 1);

      for (auto index{ 0 }; index < segmentCount; ++index)
      {
         const auto angle = slice * index - startingAngle;

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
