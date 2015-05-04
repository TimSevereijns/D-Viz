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

      /**
       * @brief IsKeyDown indicates whether the key in question is currently
       * in a pressed state (according to the keyboard manager).
       * 
       * @param[in] key                The key whose state to check.
       * 
       * @returns true if the key is currently down; false otherwise.
       */
      bool IsKeyDown(const Qt::Key key);
      
      /**
      * @brief IsKeyUp indicates whether the key in question is currently
      * not in a pressed state.
      * 
      * @param[in] key                 The key whose state to check.
      * 
      * @returns true if the key is currently up; false otherwise.
      */
      bool IsKeyUp(const Qt::Key key);

      /**
       * @brief UpdateKeyState Records the state of the specified key.
       * 
       * @param key[in]                The key to be updated.
       * @param state[in]              The state of the key.
       */
      void UpdateKeyState(const Qt::Key, const KEY_STATE state);

   private:
      std::map<Qt::Key, KEY_STATE> m_keyMap;
};

#endif // KEYBOARDMANAGER_H
