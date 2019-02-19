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
    enum class KEY_STATE { UP, DOWN };

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
    std::unordered_map<Qt::Key, KEY_STATE> m_keyMap{
        { Qt::Key_A, KEY_STATE::UP },     { Qt::Key_D, KEY_STATE::UP },
        { Qt::Key_S, KEY_STATE::UP },     { Qt::Key_W, KEY_STATE::UP },
        { Qt::Key_Shift, KEY_STATE::UP }, { Qt::Key_Control, KEY_STATE::UP }
    };
};

#endif // KEYBOARDMANAGER_H
