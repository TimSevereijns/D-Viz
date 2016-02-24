#ifndef XBOXCONTROLLER_H
#define XBOXCONTROLLER_H

#include <cstdint>
#include <functional>
#include <numeric>
#include <memory>
#include <unordered_map>

#include <QObject>
#include <QTimer>

#include "xInput.h"

/**
 * @brief The XboxController class tracks and manages the state of the attached Xbox controller(s).
 *
 * Start life as a class based on SimpleXbox360Controller by pilatomic:
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
         StateAndHandlers()
            : state(XboxController::KEY_STATE::UP),
              onButtonDown(nullptr),
              onButtonUp(nullptr)
         {
         }

         explicit StateAndHandlers(XboxController::KEY_STATE startingState)
            : state(startingState),
              onButtonDown(nullptr),
              onButtonUp(nullptr)
         {
         }

         StateAndHandlers(XboxController::KEY_STATE startingState,
            const std::function<void ()>& downHandler,
            const std::function<void ()>& upHandler)
            : state(startingState),
              onButtonDown(downHandler),
              onButtonUp(upHandler)
         {
         }

         XboxController::KEY_STATE state;
         std::function<void ()> onButtonDown;
         std::function<void ()> onButtonUp;
      };

      /**
       * @brief The State struct represents a snapshot of the state of the controller.
       */
      struct State
      {
         explicit State();

          uint8_t batteryType;
          uint8_t batteryLevel;

          uint16_t buttons;

          float leftTrigger;
          float rightTrigger;
          float leftThumbX;
          float leftThumbY;
          float rightThumbX;
          float rightThumbY;

          static bool BatteryEquals(const State& lhs, const State& rhs);

          bool operator==(const XboxController::State& rhs);
          bool operator!=(const XboxController::State& rhs);
      };

      explicit XboxController(unsigned int m_controllerNumber = 0,
         int16_t m_leftStickDeadZone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE,
         int16_t m_rightStickDeadZone = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE,
         uint8_t m_triggerThreshold = XINPUT_GAMEPAD_TRIGGER_THRESHOLD,
         QObject* parent = nullptr);

      ~XboxController();

      bool HasStateChanged(void);
      bool IsConnected(void);
      bool IsButtonDown(const unsigned int button) const;

      const XboxController::State& GetCurrentState(void);

      void SetDownHandler(const unsigned int targetButton, const std::function<void ()>& handler);
      void SetUpHandler(const unsigned int targetButton, const std::function<void ()>& handler);

   signals:
      void NewControllerState(XboxController::State);
      void NewControllerBatteryState(uint8_t newBatteryType, uint8_t newBatteryLevel);
      void ControllerConnected(unsigned int m_controllerNumber);
      void ControllerDisconnected(unsigned int m_controllerNumber);

   public slots:
      void StartAutoPolling(unsigned int interval);
      void StopAutoPolling(void);
      void Update(void);
      void SetVibration(float leftVibration, float rightVibration);

      // The following methods allows you to change the deadzones values, although the default
      // values should be fine.
      void SetLeftStickDeadZone(int16_t newDeadZone);
      void SetRightStickDeadZone(int16_t newDeadZone);
      void SetTriggerThreshold(uint8_t newThreshold);

   private:
      bool m_isCurrentControllerConnected;
      bool m_isPreviousControllerConnected;

      int m_controllerNumber;
      int m_leftStickDeadZone;
      int m_rightStickDeadZone;
      int m_triggerThreshold;

      std::unique_ptr<QTimer> m_pollingTimer;

      State m_previousState;
      State m_currentState;

      std::unordered_map<unsigned int, StateAndHandlers> m_buttonMap;
};

#endif // XBOXCONTROLLER_H

