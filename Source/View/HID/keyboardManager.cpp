#include "View/HID/keyboardManager.h"

bool KeyboardManager::IsKeyDown(const Qt::Key key)
{
    return m_keyMap[key] == KEY_STATE::DOWN;
}

bool KeyboardManager::IsKeyUp(const Qt::Key key)
{
    return m_keyMap[key] == KEY_STATE::UP;
}

void KeyboardManager::UpdateKeyState(const Qt::Key key, const KEY_STATE state)
{
    if (state == KEY_STATE::DOWN) {
        m_keyMap[key] = KeyboardManager::KEY_STATE::DOWN;
    } else if (state == KEY_STATE::UP) {
        m_keyMap[key] = KeyboardManager::KEY_STATE::UP;
    }
}
