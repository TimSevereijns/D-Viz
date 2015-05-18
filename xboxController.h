#ifndef XBOXCONTROLLER_H
#define XBOXCONTROLLER_H

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
      };

      explicit XboxController(unsigned int controllerNum = 0,
         unsigned int leftStickDeadZone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE,
         unsigned int rightStickDeadZone = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE,
         unsigned int triggerThreshold = XINPUT_GAMEPAD_TRIGGER_THRESHOLD,
         QObject* parent = 0);

      ~XboxController();

      bool isStateChanged(void);
      bool isConnected(void){return curConnected;}

      XboxController::State getCurrentState(void){return curState;}

      // TODO: Add ability to toggle key repeats.

   signals:
      void controllerNewState(XboxController::State);
      void controllerNewBatteryState(quint8 newBatteryType, quint8 newBatteryLevel);
      void controllerConnected(unsigned int controllerNum);
      void controllerDisconnected(unsigned int controllerNum);

   public slots:
      void startAutoPolling(unsigned int interval);
      void stopAutoPolling(void);
      void update(void);
      void setVibration(float leftVibration, float rightVibration);

      // The following methods allows you to change the deadzones values, although the default values
      // should be fine.
      void setLeftStickDeadZone(unsigned int newDeadZone){leftStickDeadZone=qMin(newDeadZone,MAX_STICK_VALUE);}
      void setRightStickDeadZone(unsigned int newDeadZone){rightStickDeadZone=qMin(newDeadZone,MAX_STICK_VALUE);}
      void setTriggerThreshold(unsigned int newThreshold){triggerThreshold=qMin(newThreshold,MAX_TRIGGER_VALUE);}

   private:
      static bool processStickDeadZone(qint16 rawXValue, qint16 rawYValue, float& xValue,
         float& yValue, unsigned int deadZoneRadius);

      static bool processTriggerThreshold(quint8 rawValue, float& value,unsigned int triggerThreshold);

      bool curConnected;
      bool prevConnected;

      unsigned int controllerNum;
      unsigned int leftStickDeadZone;
      unsigned int rightStickDeadZone;
      unsigned int triggerThreshold;

      QTimer* pollingTimer;

      State prevState;
      State curState;
};

#endif // XBOXCONTROLLER_H

bool operator==(XboxController::State const& a, XboxController::State const& b);
bool operator!=(XboxController::State const& a, XboxController::State const& b);

