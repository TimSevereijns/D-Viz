#include "keyboardManager.h"

#include <iostream>

KeyboardManager::KeyboardManager()
   : m_keyMap(
      {
         { Qt::Key_A, KEY_STATE::UP },
         { Qt::Key_B, KEY_STATE::UP },
         { Qt::Key_C, KEY_STATE::UP },
         { Qt::Key_D, KEY_STATE::UP },
         { Qt::Key_E, KEY_STATE::UP },
         { Qt::Key_F, KEY_STATE::UP },
         { Qt::Key_G, KEY_STATE::UP },
         { Qt::Key_H, KEY_STATE::UP },
         { Qt::Key_I, KEY_STATE::UP },
         { Qt::Key_J, KEY_STATE::UP },
         { Qt::Key_K, KEY_STATE::UP },
         { Qt::Key_L, KEY_STATE::UP },
         { Qt::Key_M, KEY_STATE::UP },
         { Qt::Key_N, KEY_STATE::UP },
         { Qt::Key_O, KEY_STATE::UP },
         { Qt::Key_P, KEY_STATE::UP },
         { Qt::Key_Q, KEY_STATE::UP },
         { Qt::Key_R, KEY_STATE::UP },
         { Qt::Key_S, KEY_STATE::UP },
         { Qt::Key_T, KEY_STATE::UP },
         { Qt::Key_U, KEY_STATE::UP },
         { Qt::Key_V, KEY_STATE::UP },
         { Qt::Key_W, KEY_STATE::UP },
         { Qt::Key_X, KEY_STATE::UP },
         { Qt::Key_Y, KEY_STATE::UP },
         { Qt::Key_Z, KEY_STATE::UP }
      })
{
}

KeyboardManager::~KeyboardManager()
{
}

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
   if (state == KEY_STATE::DOWN)
   {
      m_keyMap[key] = KeyboardManager::KEY_STATE::DOWN;
      std::cout << "Key down" << std::endl;
   }
   else if (state == KEY_STATE::UP)
   {
      m_keyMap[key] = KeyboardManager::KEY_STATE::UP;
      std::cout << "Key up" << std::endl;
   }
}

