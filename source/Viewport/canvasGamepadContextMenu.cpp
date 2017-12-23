#include "canvasGamepadContextMenu.h"

#include "../constants.h"

#include <cmath>

#include <QFontMetrics>
#include <QString>
#include <QPainter>

namespace
{
   /**
    * @brief Computes the coordinates at which we want to attach menu entries.
    *
    * @param[in] origin             The origin, or center, of the circular menu.
    * @param[in] radius             The radius of the circle that forms the menu.
    * @param[in, out] entries       The menu entries.
    */
   void ComputeLabelAttachmentPoints(
      QPoint origin,
      int radius,
      std::vector<GamepadContextMenu::Entry>& entries)
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
      const GamepadContextMenu::Entry entry,
      const QPoint& menuCenter)
   {
      auto adjustedPosition{ entry.Position };

      if (entry.Position.x() == menuCenter.x())
      {
         constexpr auto halfCharacterWidth{ 6 };
         adjustedPosition -= QPoint{ entry.Label.size() * halfCharacterWidth, 0 };
      }
      else if (entry.Position.x() < menuCenter.x())
      {
         constexpr auto presumedCharacterWidth{ 12 };
         adjustedPosition -= QPoint{ entry.Label.size() * presumedCharacterWidth, 0 };
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

GamepadContextMenu::GamepadContextMenu(QWidget* parent) :
   QWidget{ parent }
{
   setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);

   setAttribute(Qt::WA_NoSystemBackground);
   setAttribute(Qt::WA_TranslucentBackground);
   setAttribute(Qt::WA_ShowWithoutActivating);

   m_font.setFamily("Courier"); // @todo Look into using the QFontDatabase instead.
   m_font.setPointSize(16);
   m_font.setBold(true);

   m_pen.setColor(Qt::green);
   m_pen.setWidth(4);
}

void GamepadContextMenu::AddEntry(
   const QString& label,
   const std::function<void ()>& action)
{
   Entry entry{ label, QPointF{ }, action };
   m_entries.emplace_back(std::move(entry));
}

void GamepadContextMenu::ComputeLayout()
{
   const auto center = QPoint{ width() / 2, height() / 2 };

   constexpr auto radius{ 100 };
   ComputeLabelAttachmentPoints(center, 1.25 * radius, m_entries);
}

void GamepadContextMenu::paintEvent(QPaintEvent* /*event*/)
{
   m_painter.begin(this);

   const auto center = QPoint{ width() / 2, height() / 2 };

   RenderLabels(center);
   RenderGeometry(center);

   m_painter.end();
}

void GamepadContextMenu::RenderLabels(const QPoint& center)
{
   m_painter.setPen(m_pen);
   m_painter.setFont(m_font);
   m_painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
   m_painter.drawText(rect(), Qt::AlignCenter, "D-Viz");

   for (const auto& entry : m_entries)
   {
      const auto adjustedOrigin = AdjustTextOriginBasedOnLocation(entry, center);
      m_painter.drawText(adjustedOrigin, entry.Label);
   }
}

void GamepadContextMenu::RenderGeometry(const QPoint& center)
{
   constexpr auto radius{ 100 };
   m_painter.drawEllipse(center, radius, radius);
}
