#ifndef KEYBOARDMANAGER_H
#define KEYBOARDMANAGER_H

#include <unordered_map>

#include <QKeyEvent>

/**
 * @brief Tracks the state of various keys on the keyboard. This is especially useful when you want
 * to know if a particular key is being held down or not.
 */
class KeyboardManager
{
  public:
    enum class KeyState
    {
        Up,
        Down
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
    void UpdateKeyState(const Qt::Key, const KeyState state);

  private:
    std::unordered_map<Qt::Key, KeyState> m_keyMap{
        { Qt::Key_A, KeyState::Up },     { Qt::Key_D, KeyState::Up },
        { Qt::Key_S, KeyState::Up },     { Qt::Key_W, KeyState::Up },
        { Qt::Key_Shift, KeyState::Up }, { Qt::Key_Control, KeyState::Up }
    };
};

#endif // KEYBOARDMANAGER_H
