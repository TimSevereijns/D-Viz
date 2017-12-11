#ifndef GAMEPADMENUASSET_H
#define GAMEPADMENUASSET_H

#include "lineAsset.h"

#include <functional>
#include <utility>
#include <vector>

#include <QFont>
#include <QPainter>
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
          * @brief RenderText
          *
          * @param camera
          */
         void RenderText(const Camera& camera);

         /**
          * @brief Construct
          *
          * @param origin
          * @param entries
          */
         void Construct(
            QPoint origin,
            const std::vector<std::pair<QString, std::function<void ()>>>& entries);

         /**
          * @brief SetRenderContext
          *
          * @param context
          */
         void SetRenderContext(GLCanvas* context);

      private:

         void AddCircle(
            QPoint origin,
            int radius,
            int segmentCount);

         QPaintDevice* m_context{ nullptr };

         QPainter m_painter;

         QFont m_font{ "Helvetica", 30 };
   };
}

#endif // GAMEPADMENUASSET_H
