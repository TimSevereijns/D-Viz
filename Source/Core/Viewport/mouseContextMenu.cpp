#include "mouseContextMenu.h"

MouseContextMenu::MouseContextMenu(KeyboardManager& keyboardManager)
    : m_keyboardManager{ keyboardManager }
{
}

void MouseContextMenu::keyReleaseEvent(QKeyEvent* event)
{
    if (!event) {
        return;
    }

    if (event->isAutoRepeat()) {
        event->ignore();
        return;
    }

    const auto state = KeyboardManager::KEY_STATE::UP;
    m_keyboardManager.UpdateKeyState(static_cast<Qt::Key>(event->key()), state);

    event->accept();
}
