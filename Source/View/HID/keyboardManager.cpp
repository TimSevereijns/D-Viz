#include "View/HID/keyboardManager.h"

bool KeyboardManager::IsKeyDown(const Qt::Key key)
{
    return m_keyMap[key] == KeyState::Down;
}

bool KeyboardManager::IsKeyUp(const Qt::Key key)
{
    return m_keyMap[key] == KeyState::Up;
}

void KeyboardManager::UpdateKeyState(const Qt::Key key, const KeyState state)
{
    if (state == KeyState::Down) {
        m_keyMap[key] = KeyboardManager::KeyState::Down;
    } else if (state == KeyState::Up) {
        m_keyMap[key] = KeyboardManager::KeyState::Up;
    }
}
