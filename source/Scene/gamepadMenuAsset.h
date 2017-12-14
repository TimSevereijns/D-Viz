#ifndef GAMEPADMENUASSET_H
#define GAMEPADMENUASSET_H

#include "lineAsset.h"

#include <functional>
#include <utility>
#include <vector>

#include <QFont>
#include <QPainter>
#include <QPoint>
#include <QString>

class GLCanvas;

namespace Asset
{
   /**
    * @brief The GamepadMenuAsset class
    */
   class GamepadMenu final : public Line
   {
      public:

         struct Entry
         {
            QString Label;
            QPointF Position;
            std::function<void ()> Action;
         };

         /**
          * @see Asset::Base::Base(...)
          */
         GamepadMenu(
            QOpenGLExtraFunctions& openGL,
            bool isInitiallyVisible);

         /**
          * @see Asset::Base::Render(...)
          */
         bool Render(
            const Camera& camera,
            const std::vector<Light>& lights,
            const Settings::Manager& settings) override;

         /**
          * @brief Construct
          *
          * @param origin
          * @param entries
          */
         void Construct(
            const QPoint& origin,
            const std::vector<Entry>& entries);

         /**
          * @brief SetRenderContext
          *
          * @param context
          */
         void SetRenderContext(GLCanvas* context);

      private:

         void RenderLabels(const Camera& camera);

         void AddCircle(
            QPoint origin,
            int radius,
            int segmentCount);

         QPoint m_menuOrigin;

         QPaintDevice* m_context{ nullptr };

         QPainter m_painter;

         QFont m_font;

         std::vector<Entry> m_entries;
   };
}

#endif // GAMEPADMENUASSET_H
