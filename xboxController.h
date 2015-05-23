#ifndef XBOXCONTROLLER_H
#define XBOXCONTROLLER_H

#include <map>
#include <functional>

#include <QObject>
#include <QTimer>
#include <QDebug>

#include "xInput.h"

#define MAX_STICK_VALUE 32767u
#define MAX_TRIGGER_VALUE 255u
#define MAX_VIBRATION_VALUE 65535u

/**
 * @brief The XboxController class is heavily based on the SimpleXbox360Controller by pilatomic:
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

      enum class BUTTON
      {
         A,
         B,
         X,
         Y,
         LEFT_SHOULDER,
         RIGHT_SHOULDER,
         LEFT_JOYSTICK_CLICK,
         RIGHT_JOYSTICK_CLICK,
         BACK,
         START,
         DPAD_UP,
         DPAD_LEFT,
         DPAD_RIGHT,
         DPAD_DOWN
      };

      /**
       * @brief The StateAndHandlers struct
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
       * @brief The State class
       */
      class State
      {
         public:
             explicit State();

             quint8 batteryType;
             quint8 batteryLevel;

             quint16 buttons;
             bool isRepeatingKey;

             float leftTrigger;
             float rightTrigger;
             float leftThumbX;
             float leftThumbY;
             float rightThumbX;
             float rightThumbY;

             bool isButtonDown(const XboxController::BUTTON button) const;

             static bool equals(State const& a, State const& b);
             static bool batteryEquals(State const& a, State const& b);

             std::map<XboxController::BUTTON, StateAndHandlers> m_buttonMap;
      };

      explicit XboxController(unsigned int m_controllerNum = 0,
         unsigned int m_leftStickDeadZone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE,
         unsigned int m_rightStickDeadZone = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE,
         unsigned int m_triggerThreshold = XINPUT_GAMEPAD_TRIGGER_THRESHOLD,
         QObject* parent = nullptr);

      ~XboxController();

      bool HasStateChanged(void);
      bool IsConnected(void);

      XboxController::State GetCurrentState(void){return m_currentState;}

      // TODO: Add ability to toggle key repeats.

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

      QTimer* m_pollingTimer;

      State m_previousState;
      State m_currentState;
};

#endif // XBOXCONTROLLER_H

bool operator==(XboxController::State const& a, XboxController::State const& b);
bool operator!=(XboxController::State const& a, XboxController::State const& b);

