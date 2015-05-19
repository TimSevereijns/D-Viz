#include "xboxController.h"

#include <cmath>
#include <iostream>

XboxController::State::State():
   buttons(0),
   isRepeatingKey(false),
   leftTrigger(0),
   rightTrigger(0),
   leftThumbX(0),
   leftThumbY(0),
   rightThumbX(0),
   rightThumbY(0),
   batteryType(BATTERY_TYPE_DISCONNECTED),
   batteryLevel(BATTERY_LEVEL_EMPTY)
{
}

bool XboxController::State::equals(const State& a, const State& b)
{
   return (a.buttons==b.buttons)&&
          (a.leftThumbX==b.leftThumbX)&&
          (a.leftThumbY==b.leftThumbY)&&
          (a.leftTrigger==b.leftTrigger)&&
          (a.rightThumbX==b.rightThumbX)&&
          (a.rightThumbY==b.rightThumbY)&&
          (a.rightTrigger==b.rightTrigger)&&
          (a.batteryType==b.batteryType)&&
          (a.batteryLevel==b.batteryLevel);
}

bool XboxController::State::batteryEquals(const State& a, const State& b)
{
   return (a.batteryType == b.batteryType) && (a.batteryLevel == b.batteryLevel);
}

bool XboxController::State::isButtonPressed(quint16 button) const
{
   return (buttons & button);
}

bool XboxController::State::isButtonRepeating() const
{
   return isRepeatingKey;
}

XboxController::XboxController(unsigned int controllerNum, unsigned int leftStickDeadZone,
                               unsigned int rightStickDeadZone, unsigned int triggerThreshold,
                               QObject *parent)
   : QObject(parent),
     curConnected(false),
     prevConnected(false),
     leftStickDeadZone(qMin(leftStickDeadZone, MAX_STICK_VALUE)),
     rightStickDeadZone(qMin(rightStickDeadZone, MAX_STICK_VALUE)),
     triggerThreshold(qMin(triggerThreshold, MAX_TRIGGER_VALUE))
{
   this->controllerNum = qMin(controllerNum, 3u);
   pollingTimer = new QTimer();
   connect(pollingTimer, SIGNAL(timeout()), this, SLOT(update()));
}

void XboxController::startAutoPolling(unsigned int interval)
{
   pollingTimer->start(interval);
}

void XboxController::stopAutoPolling()
{
   pollingTimer->stop();
}

void XboxController::update()
{
   XINPUT_KEYSTROKE xKeyStroke;
   memset(&xKeyStroke, 0, sizeof(XINPUT_KEYSTROKE));
   const bool newKeyEvent = XInputGetKeystroke(controllerNum, XINPUT_FLAG_GAMEPAD, &xKeyStroke) != ERROR_EMPTY;

   XINPUT_STATE xInputState;
   memset(&xInputState, 0, sizeof(XINPUT_STATE));
   curConnected = (XInputGetState(controllerNum, &xInputState) == ERROR_SUCCESS);

   //handling gamepad connection/deconnection
   if (prevConnected == false && curConnected == true)
   {
      emit controllerConnected(controllerNum);
   }
   else if (prevConnected == true && curConnected == false)
   {
      emit controllerDisconnected(controllerNum);
   }

   prevConnected = curConnected;

   if (curConnected)
   {
      //buttons state
      curState.buttons = xInputState.Gamepad.wButtons;
      curState.isRepeatingKey = false;

      if (newKeyEvent && (xKeyStroke.Flags & XINPUT_KEYSTROKE_REPEAT))
      {
            std::cout << "Repeating Key" << std::endl;
      }

      //std::cout << "curState.isRepeatingKey: " << curState.isRepeatingKey << std::endl;

      //sticks state
      processStickDeadZone(xInputState.Gamepad.sThumbLX,
                           xInputState.Gamepad.sThumbLY,
                           curState.leftThumbX,
                           curState.leftThumbY,
                           leftStickDeadZone);
      processStickDeadZone(xInputState.Gamepad.sThumbRX,
                           xInputState.Gamepad.sThumbRY,
                           curState.rightThumbX,
                           curState.rightThumbY,
                           rightStickDeadZone);

      //triggers state
      processTriggerThreshold(xInputState.Gamepad.bLeftTrigger,
                              curState.leftTrigger,
                              triggerThreshold);
      processTriggerThreshold(xInputState.Gamepad.bRightTrigger,
                              curState.rightTrigger,
                              triggerThreshold);

      //battery state
      XINPUT_BATTERY_INFORMATION xInputBattery;
      memset(&xInputBattery,0,sizeof(XINPUT_BATTERY_INFORMATION));
      if (XInputGetBatteryInformation(controllerNum, BATTERY_DEVTYPE_GAMEPAD, &xInputBattery) == ERROR_SUCCESS)
      {
         curState.batteryType = xInputBattery.BatteryType;
         curState.batteryLevel = xInputBattery.BatteryLevel;
      }
      else
      {
         curState.batteryType = BATTERY_TYPE_UNKNOWN;
         curState.batteryLevel = BATTERY_LEVEL_EMPTY;
      }

      if (curState != prevState)
      {
         emit controllerNewState(curState);
      }

      if (!State::batteryEquals(prevState, curState))
      {
         emit controllerNewBatteryState(curState.batteryType, curState.batteryLevel);
      }

      prevState = curState;
   }
}

bool XboxController::processStickDeadZone(qint16 rawXValue, qint16 rawYValue,
   float& xValue, float& yValue, const unsigned int deadZoneRadius)
{
   xValue = 0;
   yValue = 0;

   //making values symetrical (otherwise negative value can be 1 unit larger than positive value)
   rawXValue = qMax(rawXValue, (qint16) - MAX_STICK_VALUE);
   rawYValue = qMax(rawYValue, (qint16) - MAX_STICK_VALUE);

   float magnitude = sqrt(rawXValue * rawXValue + rawYValue * rawYValue);
   if (magnitude < deadZoneRadius)
   {
      return false;
   }

   //remapping values to make deadzone transparent
   xValue = ((float) rawXValue) / magnitude;
   yValue = ((float) rawYValue) / magnitude;

   magnitude = qMin(magnitude, (float) MAX_STICK_VALUE);
   float normalizedMagnitude = (magnitude - deadZoneRadius) / ((MAX_STICK_VALUE - deadZoneRadius));

   xValue *= normalizedMagnitude;
   yValue *= normalizedMagnitude;

   return true;
}

bool XboxController::processTriggerThreshold(const quint8 rawValue, float& value,
   const unsigned int triggerThreshold)
{
   value = 0;
   if (rawValue < triggerThreshold)
   {
      return false;
   }

   value = ((float) rawValue - triggerThreshold) / (MAX_TRIGGER_VALUE - triggerThreshold);
   return true;
}

void XboxController::setVibration(const float leftVibration, const float rightVibration)
{
   XINPUT_VIBRATION xInputVibration;
   xInputVibration.wLeftMotorSpeed = MAX_VIBRATION_VALUE * qBound(0.0f, 1.0f, leftVibration);
   xInputVibration.wRightMotorSpeed = MAX_VIBRATION_VALUE * qBound(0.0f, 1.0f, rightVibration);
   XInputSetState(controllerNum, &xInputVibration);
}

bool XboxController::isStateChanged(void)
{
   return curState == prevState;
}

XboxController::~XboxController()
{
   setVibration(0, 0);

   pollingTimer->stop();
   delete pollingTimer;
}

bool operator==(const XboxController::State& a, const XboxController::State& b){
   return XboxController::State::equals(a,b);
}

bool operator!=(const XboxController::State& a, const XboxController::State& b){
   return !XboxController::State::equals(a, b);
}
