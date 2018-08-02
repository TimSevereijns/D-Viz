#ifndef CANVASGAMEPADCONTEXTMENU_H
#define CANVASGAMEPADCONTEXTMENU_H

#include <functional>
#include <vector>

#include <QFont>
#include <QPainter>
#include <QPoint>
#include <QTimer>
#include <QWidget>

class Gamepad;

class GamepadContextMenu : public QWidget
{
   Q_OBJECT

public:

   struct Entry
   {
      QString Label;
      QPoint Position;
      QColor Color;
      std::function<void ()> Action;
   };

   GamepadContextMenu(
      const Gamepad& gamepad,
      QWidget* parent = nullptr);

   void AddEntry(
      const QString& label,
      const std::function<void ()>& action);

   void ComputeLayout();

   void ExecuteSelection();

   void paintEvent(QPaintEvent* event) override;

private slots:

   void ProcessInput();

private:

   void RenderLabels(const QPoint& center);
   void RenderGeometry(const QPoint& center);

   const Gamepad& m_gamepad;

   std::size_t m_indexOfSelection{ std::numeric_limits<std::size_t>::max() };

   std::vector<Entry> m_entries;

   QPoint m_selectorDot;

   QTimer m_inputTimer;

   QPainter m_painter;

   QFont m_font;
   QPen m_pen;
};

#endif // CANVASGAMEPADCONTEXTMENU_H
