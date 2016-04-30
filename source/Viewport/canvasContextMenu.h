#ifndef CANVASCONTEXTMENU_H
#define CANVASCONTEXTMENU_H

#include "../HID/keyboardManager.h"

#include <QMenu>

class CanvasContextMenu : public QMenu
{
   public:
      CanvasContextMenu(KeyboardManager& keyboardManager);

   protected:
      /**
       * @note Overriding this function ensures that we'll still be able to properly track the
       * release of the control keys, even when that release occurs while the context menu is open.
       * Without this override, if the user released the control key while the context menu is still
       * open, we would never be notified of this release due to the modal nature of the menu.
       */
      void keyReleaseEvent(QKeyEvent* event) override;

   private:
      KeyboardManager& m_keyboardManager;
};

#endif // CANVASCONTEXTMENU_H
