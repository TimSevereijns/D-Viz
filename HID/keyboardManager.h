#ifndef KEYBOARDMANAGER_H
#define KEYBOARDMANAGER_H

#include <unordered_map>

#include <QKeyEvent>

/**
 * @brief The KeyboardManager class tracks the state of various keys on the keyboard. This is
 * especially useful when you want to know if a particular key is being held down or not.
 */
class KeyboardManager
{
   public:
      enum class KEY_STATE
      {
         UP,
         DOWN
      };

      /**
       * @brief Indicates whether the key in question is currently in a pressed state (according to
       * the keyboard manager).
       * 
       * @param[in] key                The key whose state to check.
       * 
       * @returns True if the key is currently down; false otherwise.
       */
      bool IsKeyDown(const Qt::Key key);
      
      /**
      * @brief Indicates whether the key in question is currently not in a pressed state.
      * 
      * @param[in] key                 The key whose state to check.
      * 
      * @returns True if the key is currently up; false otherwise.
      */
      bool IsKeyUp(const Qt::Key key);

      /**
       * @brief Records the state of the specified key.
       * 
       * @param key[in]                The key to be updated.
       * @param state[in]              The state of the key.
       */
      void UpdateKeyState(const Qt::Key, const KEY_STATE state);

   private:
      std::unordered_map<Qt::Key, KEY_STATE> m_keyMap
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
         { Qt::Key_Z, KEY_STATE::UP },
         { Qt::Key_Shift, KEY_STATE::UP }
      };
};

#endif // KEYBOARDMANAGER_H
