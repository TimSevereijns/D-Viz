#ifndef CANVASGAMEPADCONTEXTMENU_H
#define CANVASGAMEPADCONTEXTMENU_H

#include <functional>
#include <vector>

#include <QFont>
#include <QPainter>
#include <QWidget>

class GamepadContextMenu : public QWidget
{
   public:

      struct Entry
      {
         QString Label;
         QPointF Position;
         std::function<void ()> Action;
      };

      explicit GamepadContextMenu(QWidget* parent = nullptr);

      void AddEntry(
         const QString& label,
         const std::function<void ()>& action);

      void ComputeLayout();

      void paintEvent(QPaintEvent* event) override;

   private:

      void RenderLabels(const QPoint& center);
      void RenderGeometry(const QPoint& center);

      std::vector<Entry> m_entries;

      QPainter m_painter;

      QFont m_font;
      QPen m_pen;
};

#endif // CANVASGAMEPADCONTEXTMENU_H
