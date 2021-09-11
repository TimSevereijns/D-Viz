#ifndef XBOXGAMEPAD_H
#define XBOXGAMEPAD_H

#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <numeric>
#include <unordered_map>

#include <QObject>
#include <QTimer>

#include "constants.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <Xinput.h>

/**
 * @brief The XboxController class tracks and manages the state of the attached Xbox controller(s).
 *
 * Inspired by Pilatomic's SimpleXbox360Controller:
 * https://www.gitorious.org/simple-xbox-360-controller/pages/Home
 */
class XboxGamepad : public QObject
{
    Q_OBJECT

  public:
    constexpr static std::uint8_t MaxTriggerValue = std::numeric_limits<std::int8_t>::max();
    constexpr static std::uint8_t MinTriggerValue = std::numeric_limits<std::int8_t>::min();

    constexpr static std::int16_t MaxStickValue = std::numeric_limits<std::int16_t>::max();
    constexpr static std::int16_t MinStickValue = std::numeric_limits<std::int16_t>::min();

    constexpr static std::int32_t MaxVibrationValue = std::numeric_limits<std::int32_t>::max();
    constexpr static std::int32_t MinVibrationValue = std::numeric_limits<std::int32_t>::min();

    enum class KeyState
    {
        Up,
        Down
    };

    /**
     * @brief Contains a key's current button state (either up or down), and the event handlers that
     * deal with a key going down or coming up.
     */
    struct StateAndHandlers
    {
        StateAndHandlers() = default;

        explicit StateAndHandlers(XboxGamepad::KeyState startingState) : state{ startingState }
        {
        }

        StateAndHandlers(
            XboxGamepad::KeyState startingState, const std::function<void()>& downHandler,
            const std::function<void()>& upHandler)
            : state{ startingState }, onButtonDown{ downHandler }, onButtonUp{ upHandler }
        {
        }

        XboxGamepad::KeyState state = XboxGamepad::KeyState::Up;

        std::function<void()> onButtonDown;
        std::function<void()> onButtonUp;
    };

    /**
     * @brief Represents a snapshot of the state of the controller.
     */
    struct State
    {
        std::uint16_t buttons = 0;

        float leftTrigger = 0.0f;
        float rightTrigger = 0.0f;
        float leftThumbX = 0.0f;
        float leftThumbY = 0.0f;
        float rightThumbX = 0.0f;
        float rightThumbY = 0.0f;

        bool operator==(const XboxGamepad::State& rhs) const;
        bool operator!=(const XboxGamepad::State& rhs) const;
    };

    XboxGamepad(int m_controllerNumber = 0, QObject* parent = nullptr);

    ~XboxGamepad();

    bool HasStateChanged() const;

    bool IsConnected() const;

    bool IsButtonDown(unsigned int button) const;

    bool buttonUp() const
    {
        return IsButtonDown(XINPUT_GAMEPAD_DPAD_UP);
    }

    bool buttonLeft() const
    {
        return IsButtonDown(XINPUT_GAMEPAD_DPAD_LEFT);
    }

    bool buttonRight() const
    {
        return IsButtonDown(XINPUT_GAMEPAD_DPAD_RIGHT);
    }

    bool buttonDown() const
    {
        return IsButtonDown(XINPUT_GAMEPAD_DPAD_DOWN);
    }

    bool buttonL1() const
    {
        return IsButtonDown(XINPUT_GAMEPAD_LEFT_SHOULDER);
    }

    bool buttonR1() const
    {
        return IsButtonDown(XINPUT_GAMEPAD_RIGHT_SHOULDER);
    }

    float buttonR2() const
    {
        return m_currentState.rightTrigger;
    }

    float buttonL2() const
    {
        return m_currentState.leftTrigger;
    }

    bool buttonA() const
    {
        return IsButtonDown(XINPUT_GAMEPAD_A);
    }

    bool buttonB() const
    {
        return IsButtonDown(XINPUT_GAMEPAD_B);
    }

    bool buttonX() const
    {
        return IsButtonDown(XINPUT_GAMEPAD_X);
    }

    bool buttonY() const
    {
        return IsButtonDown(XINPUT_GAMEPAD_Y);
    }

    double axisRightX() const
    {
        return m_currentState.rightThumbX;
    }

    double axisRightY() const
    {
        return m_currentState.rightThumbY;
    }

    double axisLeftX() const
    {
        return m_currentState.leftThumbX;
    }

    double axisLeftY() const
    {
        return m_currentState.leftThumbY;
    }

    bool IsLeftTriggerDown() const
    {
        return buttonL2() >= Constants::Input::TriggerActuationThreshold;
    }

    bool IsRightTriggerDown() const
    {
        return buttonR2() >= Constants::Input::TriggerActuationThreshold;
    }

    const XboxGamepad::State& GetCurrentState() const;

    void SetDownHandler(const unsigned int targetButton, const std::function<void()>& handler);
    void SetUpHandler(const unsigned int targetButton, const std::function<void()>& handler);

  signals:
    void NewControllerState(XboxGamepad::State state);
    void ControllerConnected(unsigned int m_controllerNumber);
    void ControllerDisconnected(unsigned int m_controllerNumber);

  public slots:
    void StartAutoPolling(unsigned int interval);
    void StopAutoPolling();
    void Update();
    void SetVibration(float leftVibration, float rightVibration);

    // The following methods allows you to change the deadzones values, although the default
    // values should be fine.
    void SetLeftStickDeadZone(std::int16_t newDeadZone);
    void SetRightStickDeadZone(std::int16_t newDeadZone);
    void SetTriggerThreshold(std::uint8_t newThreshold);

  private:
    bool m_isCurrentControllerConnected = false;
    bool m_isPreviousControllerConnected = false;

    int m_controllerNumber = 0;
    int m_leftStickDeadZone = XboxGamepad::MaxStickValue;
    int m_rightStickDeadZone = XboxGamepad::MaxStickValue;
    int m_triggerThreshold = XboxGamepad::MaxTriggerValue;

    std::unique_ptr<QTimer> m_pollingTimer;

    State m_previousState;
    State m_currentState;

    std::unordered_map<unsigned int, StateAndHandlers> m_buttonMap = {
        { XINPUT_GAMEPAD_A, StateAndHandlers(XboxGamepad::KeyState::Up) },
        { XINPUT_GAMEPAD_B, StateAndHandlers(XboxGamepad::KeyState::Up) },
        { XINPUT_GAMEPAD_X, StateAndHandlers(XboxGamepad::KeyState::Up) },
        { XINPUT_GAMEPAD_Y, StateAndHandlers(XboxGamepad::KeyState::Up) },
        { XINPUT_GAMEPAD_LEFT_SHOULDER, StateAndHandlers(XboxGamepad::KeyState::Up) },
        { XINPUT_GAMEPAD_RIGHT_SHOULDER, StateAndHandlers(XboxGamepad::KeyState::Up) },
        { XINPUT_GAMEPAD_LEFT_THUMB, StateAndHandlers(XboxGamepad::KeyState::Up) },
        { XINPUT_GAMEPAD_RIGHT_THUMB, StateAndHandlers(XboxGamepad::KeyState::Up) },
        { XINPUT_GAMEPAD_BACK, StateAndHandlers(XboxGamepad::KeyState::Up) },
        { XINPUT_GAMEPAD_START, StateAndHandlers(XboxGamepad::KeyState::Up) },
        { XINPUT_GAMEPAD_DPAD_UP, StateAndHandlers(XboxGamepad::KeyState::Up) },
        { XINPUT_GAMEPAD_DPAD_LEFT, StateAndHandlers(XboxGamepad::KeyState::Up) },
        { XINPUT_GAMEPAD_DPAD_RIGHT, StateAndHandlers(XboxGamepad::KeyState::Up) },
        { XINPUT_GAMEPAD_DPAD_DOWN, StateAndHandlers(XboxGamepad::KeyState::Up) }
    };
};

#endif // XBOXGAMEPAD_H
