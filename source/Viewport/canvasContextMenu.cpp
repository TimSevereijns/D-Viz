#include "canvasContextMenu.h"

#include <assert.h>

CanvasContextMenu::CanvasContextMenu(KeyboardManager& keyboardManager)
   : m_keyboardManager{ keyboardManager }
{
}

void CanvasContextMenu::keyReleaseEvent(QKeyEvent* event)
{
   assert(event);
   if (!event || event->key() != Qt::Key_Control)
   {
      return;
   }

   if (event->isAutoRepeat())
   {
      event->ignore();
      return;
   }

   const auto state = KeyboardManager::KEY_STATE::UP;
   m_keyboardManager.UpdateKeyState(static_cast<Qt::Key>(event->key()), state);

   event->accept();
}
