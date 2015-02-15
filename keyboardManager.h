#ifndef KEYBOARDMANAGER_H
#define KEYBOARDMANAGER_H

#include <map>

#include <QKeyEvent>

class KeyboardManager
{
   public:
      enum class KEY_STATE
      {
         UP,
         DOWN
      };

      KeyboardManager();
      ~KeyboardManager();

      bool IsKeyDown(const Qt::Key key);
      bool IsKeyUp(const Qt::Key key);

      /**
       * @brief UpdateKeyState Records the state of the specified key.
       * @param key[in]                The key to be updated.
       * @param state[in]              The state of the key.
       */
      void UpdateKeyState(const Qt::Key, const KEY_STATE state);

   private:
      std::map<Qt::Key, KEY_STATE> m_keyMap;
};

#endif // KEYBOARDMANAGER_H
