#include "gamepadContextMenu.h"

#include "../constants.h"

#include "../HID/gamepad.h"

#include <cmath>
#include <iostream>

#include <QFontMetrics>
#include <QLine>
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
    * @param[in] fontMetrics        The metrics for the font in which the labels are to be rendered.
    *
    * @returns The adjusted 2D origin for the menu entry.
    */
   auto AdjustTextOriginBasedOnLocation(
      const GamepadContextMenu::Entry entry,
      const QPoint& menuCenter,
      const QFontMetrics& fontMetrics)
   {
      auto adjustedPosition{ entry.Position };

      if (entry.Position.x() == menuCenter.x())
      {
         const auto halfLabelWidth = fontMetrics.width(entry.Label) / 2;
         adjustedPosition -= QPoint{ halfLabelWidth, 0 };
      }
      else if (entry.Position.x() < menuCenter.x())
      {
         const auto fullLabelWidth = fontMetrics.width(entry.Label);
         adjustedPosition -= QPoint{ fullLabelWidth, 0 };
      }

      if (entry.Position.y() == menuCenter.y())
      {
         const auto halfLabelHeight = fontMetrics.height() / 2;
         adjustedPosition += QPoint{ 0, halfLabelHeight };
      }
      else if (entry.Position.y() > menuCenter.y())
      {
         const auto fullLabelHeight = fontMetrics.height();
         adjustedPosition += QPoint{ 0, fullLabelHeight };
      }

      return adjustedPosition;
   }

   /**
    * @brief Computes the linear distance between two points.
    *
    * @param[in] start              The first point.
    * @param[in] end                The second point.
    *
    * @returns The linear distance between two points.
    */
   auto Distance(
      const QPoint& start,
      const QPoint& end)
   {
       const auto xDelta = end.x() - start.x();
       const auto yDelta = end.y() - start.y();

       return std::sqrt(xDelta * xDelta + yDelta * yDelta);
   }

}

GamepadContextMenu::GamepadContextMenu(
   const Gamepad& gamepad,
   QWidget* parent)
   :
   QWidget{ parent },
   m_gamepad{ gamepad }
{
   setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);

   setAttribute(Qt::WA_NoSystemBackground);
   setAttribute(Qt::WA_TranslucentBackground);
   setAttribute(Qt::WA_ShowWithoutActivating);
   setAttribute(Qt::WA_DeleteOnClose);

   m_font.setFamily("Courier"); //< @todo Look into using the QFontDatabase instead.
   m_font.setPointSize(16);
   m_font.setBold(true);

   m_pen.setColor(Qt::green);
   m_pen.setWidth(4);

   connect(&m_inputTimer, &QTimer::timeout, this, &GamepadContextMenu::ProcessInput);
   m_inputTimer.start(Constants::Graphics::DESIRED_TIME_BETWEEN_FRAMES);
}

void GamepadContextMenu::ProcessInput()
{
   constexpr auto radius{ 100 };

   const auto x = m_gamepad.axisLeftX();
   const auto y = m_gamepad.axisLeftY();

   m_selectorDot = QPoint
   {
     static_cast<int>(x * radius) + width() / 2,
     static_cast<int>(y * radius) + height() / 2
   };

   const auto selection = std::find_if(std::begin(m_entries), std::end(m_entries),
      [&] (const auto& entry) noexcept
   {
      constexpr auto reasonableDistanceToLabel{ 64 };
      return Distance(m_selectorDot, entry.Position) < reasonableDistanceToLabel;
   });

   if (selection != std::end(m_entries))
   {
      if (m_indexOfSelection < m_entries.size())
      {
         m_entries[m_indexOfSelection].Color = Qt::green;
      }

      selection->Color = Qt::white;
      m_indexOfSelection = std::distance(std::begin(m_entries), selection);
   }

   repaint();
}

void GamepadContextMenu::AddEntry(
   const QString& label,
   const std::function<void ()>& action)
{
   Entry entry{ label, QPoint{ }, QColor{ Qt::green }, action };
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

   m_pen.setColor(Qt::green);
   m_painter.setPen(m_pen);

   m_painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

   const auto center = QPoint{ width() / 2, height() / 2 };

   RenderLabels(center);
   RenderGeometry(center);

   m_painter.end();
}

void GamepadContextMenu::RenderLabels(const QPoint& center)
{
   m_painter.setFont(m_font);
   m_painter.setBrush(QColor{ 0, 0, 0, 128 });

   const QFontMetrics fontMetrics{ m_font };

   for (const auto& entry : m_entries)
   {
      const auto origin = AdjustTextOriginBasedOnLocation(entry, center, fontMetrics);

      auto labelRect = fontMetrics.boundingRect(entry.Label);
      labelRect.moveTo(origin.x() - 4, origin.y() - fontMetrics.ascent() + 1);

      m_painter.setPen(Qt::NoPen);
      m_painter.drawRect(labelRect);

      m_pen.setColor(entry.Color);
      m_painter.setPen(m_pen);
      m_painter.drawText(origin, entry.Label);
   }

   m_painter.setBrush(Qt::NoBrush);
}

void GamepadContextMenu::RenderGeometry(const QPoint& center)
{
   m_pen.setColor(Qt::green);
   m_painter.setPen(m_pen);

   constexpr auto radius{ 100 };
   m_painter.drawEllipse(center, radius, radius);

   m_pen.setColor(Qt::green);
   m_painter.setPen(m_pen);

   m_painter.drawEllipse(m_selectorDot, 10, 10);
}

void GamepadContextMenu::ExecuteSelection()
{
   if (m_entries.size() < m_indexOfSelection)
   {
      return;
   }

   m_entries[m_indexOfSelection].Action();
}
