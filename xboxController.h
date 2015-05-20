#ifndef XBOXCONTROLLER_H
#define XBOXCONTROLLER_H

#include <map>

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

      enum class BINARY_BUTTON
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
         START
      };

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

             bool isButtonPressed(quint16 button) const;
             bool isButtonRepeating() const;

             static bool equals(State const& a, State const& b);
             static bool batteryEquals(State const& a, State const& b);

             std::map<XboxController::BINARY_BUTTON, XboxController::KEY_STATE> m_buttonMap;
      };

      explicit XboxController(unsigned int m_controllerNum = 0,
         unsigned int m_leftStickDeadZone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE,
         unsigned int m_rightStickDeadZone = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE,
         unsigned int m_triggerThreshold = XINPUT_GAMEPAD_TRIGGER_THRESHOLD,
         QObject* parent = 0);

      ~XboxController();

      bool isStateChanged(void);
      bool isConnected(void){return curConnected;}

      XboxController::State getCurrentState(void){return m_currentState;}

      // TODO: Add ability to toggle key repeats.

   signals:
      void controllerNewState(XboxController::State);
      void controllerNewBatteryState(quint8 newBatteryType, quint8 newBatteryLevel);
      void controllerConnected(unsigned int m_controllerNum);
      void controllerDisconnected(unsigned int m_controllerNum);

   public slots:
      void startAutoPolling(unsigned int interval);
      void stopAutoPolling(void);
      void update(void);
      void setVibration(float leftVibration, float rightVibration);

      // The following methods allows you to change the deadzones values, although the default values
      // should be fine.
      void setLeftStickDeadZone(unsigned int newDeadZone){m_leftStickDeadZone=qMin(newDeadZone,MAX_STICK_VALUE);}
      void setRightStickDeadZone(unsigned int newDeadZone){m_rightStickDeadZone=qMin(newDeadZone,MAX_STICK_VALUE);}
      void setTriggerThreshold(unsigned int newThreshold){m_triggerThreshold=qMin(newThreshold,MAX_TRIGGER_VALUE);}

   private:
      static bool processStickDeadZone(qint16 rawXValue, qint16 rawYValue, float& xValue,
         float& yValue, unsigned int deadZoneRadius);

      static bool processTriggerThreshold(quint8 rawValue, float& value,unsigned int m_triggerThreshold);

      bool curConnected;
      bool prevConnected;

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

