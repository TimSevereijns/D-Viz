#ifndef XBOXCONTROLLER_H
#define XBOXCONTROLLER_H

#include <functional>
#include <map>
#include <memory>

#include <QObject>
#include <QTimer>
#include <QDebug>

#include "xInput.h"

#define MAX_STICK_VALUE 32767u
#define MAX_TRIGGER_VALUE 255u
#define MAX_VIBRATION_VALUE 65535u

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
       * @brief The State class represents a snapshot of the state of the controller.
       */
      class State
      {
         public:
             explicit State();

             quint8 batteryType;
             quint8 batteryLevel;

             quint16 buttons;

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

      explicit XboxController(unsigned int m_controllerNum = 0,
         unsigned int m_leftStickDeadZone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE,
         unsigned int m_rightStickDeadZone = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE,
         unsigned int m_triggerThreshold = XINPUT_GAMEPAD_TRIGGER_THRESHOLD,
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
      void NewControllerBatteryState(quint8 newBatteryType, quint8 newBatteryLevel);
      void ControllerConnected(unsigned int m_controllerNum);
      void ControllerDisconnected(unsigned int m_controllerNum);

   public slots:
      void StartAutoPolling(unsigned int interval);
      void StopAutoPolling(void);
      void Update(void);
      void SetVibration(float leftVibration, float rightVibration);

      // The following methods allows you to change the deadzones values, although the default
      // values should be fine.
      void SetLeftStickDeadZone(unsigned int newDeadZone);
      void SetRightStickDeadZone(unsigned int newDeadZone);
      void SetTriggerThreshold(unsigned int newThreshold);

   private:
      bool m_isCurrentControllerConnected;
      bool m_isPreviousControllerConnected;

      unsigned int m_controllerNum;
      unsigned int m_leftStickDeadZone;
      unsigned int m_rightStickDeadZone;
      unsigned int m_triggerThreshold;

      std::unique_ptr<QTimer> m_pollingTimer;

      State m_previousState;
      State m_currentState;

      std::map<unsigned int, StateAndHandlers> m_buttonMap;
};

#endif // XBOXCONTROLLER_H

