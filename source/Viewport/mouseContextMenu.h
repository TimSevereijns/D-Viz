#ifndef CANVASCONTEXTMENU_H
#define CANVASCONTEXTMENU_H

#include "../HID/keyboardManager.h"

#include <QMenu>

class MouseContextMenu final : public QMenu
{
   public:

      explicit MouseContextMenu(KeyboardManager& keyboardManager);

   protected:

      /**
       * @note Overriding this function ensures that we'll still be able to properly track the
       * release of the keys, even when that release occurs when the context menu is open.
       * Without this override, if the user released a key while the context menu is still
       * open, we would never be notified of this release due to the modal nature of the menu.
       */
      void keyReleaseEvent(QKeyEvent* event) override;

   private:

      KeyboardManager& m_keyboardManager;
};

#endif // CANVASCONTEXTMENU_H
