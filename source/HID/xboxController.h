#ifndef XBOXCONTROLLER_H
#define XBOXCONTROLLER_H

#include <cstdint>
#include <functional>
#include <numeric>
#include <memory>
#include <unordered_map>

#include <QObject>
#include <QTimer>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <Xinput.h>

/**
 * @brief The XboxController class tracks and manages the state of the attached Xbox controller(s).
 *
 * Inspired by Pilatomic's SimpleXbox360Controller:
 * https://www.gitorious.org/simple-xbox-360-controller/pages/Home
 */
class XboxController : public QObject
{
   Q_OBJECT

   public:

      static const uint8_t MAX_TRIGGER_VALUE;
      static const uint8_t MIN_TRIGGER_VALUE;

      static const int16_t MAX_STICK_VALUE;
      static const int16_t MIN_STICK_VALUE;

      static const int32_t MAX_VIBRATION_VALUE;
      static const int32_t MIN_VIBRATION_VALUE;

      enum class KEY_STATE
      {
         UP,
         DOWN
      };

      /**
       * @brief The StateAndHandlers struct contains a key's current button state (either up or down),
       * and the event handlers that deal with a key going down or coming up.
       */
      struct StateAndHandlers
      {
         StateAndHandlers() = default;

         explicit StateAndHandlers(XboxController::KEY_STATE startingState) :
            state(startingState)
         {
         }

         StateAndHandlers(
            XboxController::KEY_STATE startingState,
            const std::function<void ()>& downHandler,
            const std::function<void ()>& upHandler)
            :
            state(startingState),
            onButtonDown(downHandler),
            onButtonUp(upHandler)
         {
         }

         XboxController::KEY_STATE state{ XboxController::KEY_STATE::UP };

         std::function<void ()> onButtonDown{ nullptr };
         std::function<void ()> onButtonUp{ nullptr };
      };

      /**
       * @brief The State struct represents a snapshot of the state of the controller.
       */
      struct State
      {
          uint16_t buttons{ 0 };

          float leftTrigger{ 0.0f };
          float rightTrigger{ 0.0f };
          float leftThumbX{ 0.0f };
          float leftThumbY{ 0.0f };
          float rightThumbX{ 0.0f };
          float rightThumbY{ 0.0f };

          bool operator==(const XboxController::State& rhs) const;
          bool operator!=(const XboxController::State& rhs) const;
      };

      XboxController(
         int m_controllerNumber = 0,
         int16_t m_leftStickDeadZone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE,
         int16_t m_rightStickDeadZone = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE,
         uint8_t m_triggerThreshold = XINPUT_GAMEPAD_TRIGGER_THRESHOLD,
         QObject* parent = nullptr);

      ~XboxController();

      bool HasStateChanged() const;
      bool IsConnected() const;
      bool IsButtonDown(const unsigned int button) const;

      const XboxController::State& GetCurrentState() const;

      void SetDownHandler(const unsigned int targetButton, const std::function<void ()>& handler);
      void SetUpHandler(const unsigned int targetButton, const std::function<void ()>& handler);

   signals:

      void NewControllerState(XboxController::State state);
      void ControllerConnected(unsigned int m_controllerNumber);
      void ControllerDisconnected(unsigned int m_controllerNumber);

   public slots:

      void StartAutoPolling(unsigned int interval);
      void StopAutoPolling();
      void Update();
      void SetVibration(float leftVibration, float rightVibration);

      // The following methods allows you to change the deadzones values, although the default
      // values should be fine.
      void SetLeftStickDeadZone(int16_t newDeadZone);
      void SetRightStickDeadZone(int16_t newDeadZone);
      void SetTriggerThreshold(uint8_t newThreshold);

   private:

      bool m_isCurrentControllerConnected{ false };
      bool m_isPreviousControllerConnected{ false };

      int m_controllerNumber{ 0 };
      int m_leftStickDeadZone{ XboxController::MAX_STICK_VALUE };
      int m_rightStickDeadZone{ XboxController::MAX_STICK_VALUE };
      int m_triggerThreshold{ XboxController::MAX_TRIGGER_VALUE };

      std::unique_ptr<QTimer> m_pollingTimer;

      State m_previousState;
      State m_currentState;

      std::unordered_map<unsigned int, StateAndHandlers> m_buttonMap
      {
         { XINPUT_GAMEPAD_A, StateAndHandlers(XboxController::KEY_STATE::UP) },
         { XINPUT_GAMEPAD_B, StateAndHandlers(XboxController::KEY_STATE::UP) },
         { XINPUT_GAMEPAD_X, StateAndHandlers(XboxController::KEY_STATE::UP) },
         { XINPUT_GAMEPAD_Y, StateAndHandlers(XboxController::KEY_STATE::UP) },
         { XINPUT_GAMEPAD_LEFT_SHOULDER, StateAndHandlers(XboxController::KEY_STATE::UP) },
         { XINPUT_GAMEPAD_RIGHT_SHOULDER, StateAndHandlers(XboxController::KEY_STATE::UP) },
         { XINPUT_GAMEPAD_LEFT_THUMB, StateAndHandlers(XboxController::KEY_STATE::UP) },
         { XINPUT_GAMEPAD_RIGHT_THUMB, StateAndHandlers(XboxController::KEY_STATE::UP) },
         { XINPUT_GAMEPAD_BACK, StateAndHandlers(XboxController::KEY_STATE::UP) },
         { XINPUT_GAMEPAD_START, StateAndHandlers(XboxController::KEY_STATE::UP) },
         { XINPUT_GAMEPAD_DPAD_UP, StateAndHandlers(XboxController::KEY_STATE::UP) },
         { XINPUT_GAMEPAD_DPAD_LEFT, StateAndHandlers(XboxController::KEY_STATE::UP) },
         { XINPUT_GAMEPAD_DPAD_RIGHT, StateAndHandlers(XboxController::KEY_STATE::UP) },
         { XINPUT_GAMEPAD_DPAD_DOWN, StateAndHandlers(XboxController::KEY_STATE::UP) }
      };
};

#endif // XBOXCONTROLLER_H
