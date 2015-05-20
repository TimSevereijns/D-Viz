#include "xboxController.h"

#include <cmath>
#include <iostream>

namespace
{
   /**
    * @brief ButtonUpdateHelper
    *
    * @param buttonStates
    * @param buttonMap
    */
   void ButtonUpdateHelper(const WORD buttonStates,
      std::map<XboxController::BINARY_BUTTON, XboxController::KEY_STATE>& buttonMap)
   {
      buttonMap[XboxController::BINARY_BUTTON::A] = (buttonStates & XINPUT_GAMEPAD_A)
         ? XboxController::KEY_STATE::DOWN
         : XboxController::KEY_STATE::UP;
      buttonMap[XboxController::BINARY_BUTTON::B] = (buttonStates & XINPUT_GAMEPAD_B)
         ? XboxController::KEY_STATE::DOWN
         : XboxController::KEY_STATE::UP;
      buttonMap[XboxController::BINARY_BUTTON::X] = (buttonStates & XINPUT_GAMEPAD_X)
         ? XboxController::KEY_STATE::DOWN
         : XboxController::KEY_STATE::UP;
      buttonMap[XboxController::BINARY_BUTTON::Y] = (buttonStates & XINPUT_GAMEPAD_Y)
         ? XboxController::KEY_STATE::DOWN
         : XboxController::KEY_STATE::UP;
      buttonMap[XboxController::BINARY_BUTTON::LEFT_SHOULDER] =
         (buttonStates & XINPUT_GAMEPAD_LEFT_SHOULDER)
         ? XboxController::KEY_STATE::DOWN
         : XboxController::KEY_STATE::UP;
      buttonMap[XboxController::BINARY_BUTTON::RIGHT_SHOULDER] =
         (buttonStates & XINPUT_GAMEPAD_RIGHT_SHOULDER)
         ? XboxController::KEY_STATE::DOWN
         : XboxController::KEY_STATE::UP;
      buttonMap[XboxController::BINARY_BUTTON::LEFT_JOYSTICK_CLICK] =
         (buttonStates & XINPUT_GAMEPAD_LEFT_THUMB)
         ? XboxController::KEY_STATE::DOWN
         : XboxController::KEY_STATE::UP;
      buttonMap[XboxController::BINARY_BUTTON::RIGHT_JOYSTICK_CLICK] =
         (buttonStates & XINPUT_GAMEPAD_RIGHT_THUMB)
         ? XboxController::KEY_STATE::DOWN
         : XboxController::KEY_STATE::UP;
      buttonMap[XboxController::BINARY_BUTTON::BACK] = (buttonStates & XINPUT_GAMEPAD_BACK)
         ? XboxController::KEY_STATE::DOWN
         : XboxController::KEY_STATE::UP;
      buttonMap[XboxController::BINARY_BUTTON::START] = (buttonStates & XINPUT_GAMEPAD_START)
         ? XboxController::KEY_STATE::DOWN
         : XboxController::KEY_STATE::UP;
   }
}

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
   batteryLevel(BATTERY_LEVEL_EMPTY),
   m_buttonMap(
      {
         { XboxController::BINARY_BUTTON::A, XboxController::KEY_STATE::UP },
         { XboxController::BINARY_BUTTON::B, XboxController::KEY_STATE::UP },
         { XboxController::BINARY_BUTTON::X, XboxController::KEY_STATE::UP },
         { XboxController::BINARY_BUTTON::Y, XboxController::KEY_STATE::UP },
         { XboxController::BINARY_BUTTON::LEFT_SHOULDER, XboxController::KEY_STATE::UP },
         { XboxController::BINARY_BUTTON::RIGHT_SHOULDER, XboxController::KEY_STATE::UP },
         { XboxController::BINARY_BUTTON::LEFT_JOYSTICK_CLICK, XboxController::KEY_STATE::UP },
         { XboxController::BINARY_BUTTON::RIGHT_JOYSTICK_CLICK, XboxController::KEY_STATE::UP },
         { XboxController::BINARY_BUTTON::BACK, XboxController::KEY_STATE::UP },
         { XboxController::BINARY_BUTTON::START, XboxController::KEY_STATE::UP }
      })
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
     m_leftStickDeadZone(qMin(leftStickDeadZone, MAX_STICK_VALUE)),
     m_rightStickDeadZone(qMin(rightStickDeadZone, MAX_STICK_VALUE)),
     m_triggerThreshold(qMin(triggerThreshold, MAX_TRIGGER_VALUE))
{
   this->m_controllerNum = qMin(controllerNum, 3u);
   m_pollingTimer = new QTimer();
   connect(m_pollingTimer, SIGNAL(timeout()), this, SLOT(update()));
}

void XboxController::startAutoPolling(unsigned int interval)
{
   m_pollingTimer->start(interval);
}

void XboxController::stopAutoPolling()
{
   m_pollingTimer->stop();
}

void XboxController::update()
{
   XINPUT_STATE xInputState;
   memset(&xInputState, 0, sizeof(XINPUT_STATE));
   curConnected = (XInputGetState(m_controllerNum, &xInputState) == ERROR_SUCCESS);

   ButtonUpdateHelper(xInputState.Gamepad.wButtons, m_currentState.m_buttonMap);

   //handling gamepad connection/deconnection
   if (prevConnected == false && curConnected == true)
   {
      emit controllerConnected(m_controllerNum);
   }
   else if (prevConnected == true && curConnected == false)
   {
      emit controllerDisconnected(m_controllerNum);
   }

   prevConnected = curConnected;

   if (curConnected)
   {
      //buttons state
      m_currentState.buttons = xInputState.Gamepad.wButtons;

      //sticks state
      processStickDeadZone(xInputState.Gamepad.sThumbLX,
                           xInputState.Gamepad.sThumbLY,
                           m_currentState.leftThumbX,
                           m_currentState.leftThumbY,
                           m_leftStickDeadZone);
      processStickDeadZone(xInputState.Gamepad.sThumbRX,
                           xInputState.Gamepad.sThumbRY,
                           m_currentState.rightThumbX,
                           m_currentState.rightThumbY,
                           m_rightStickDeadZone);

      //triggers state
      processTriggerThreshold(xInputState.Gamepad.bLeftTrigger,
                              m_currentState.leftTrigger,
                              m_triggerThreshold);
      processTriggerThreshold(xInputState.Gamepad.bRightTrigger,
                              m_currentState.rightTrigger,
                              m_triggerThreshold);

      //battery state
      XINPUT_BATTERY_INFORMATION xInputBattery;
      memset(&xInputBattery,0,sizeof(XINPUT_BATTERY_INFORMATION));
      if (XInputGetBatteryInformation(m_controllerNum, BATTERY_DEVTYPE_GAMEPAD, &xInputBattery) == ERROR_SUCCESS)
      {
         m_currentState.batteryType = xInputBattery.BatteryType;
         m_currentState.batteryLevel = xInputBattery.BatteryLevel;
      }
      else
      {
         m_currentState.batteryType = BATTERY_TYPE_UNKNOWN;
         m_currentState.batteryLevel = BATTERY_LEVEL_EMPTY;
      }

      // TODO: This seems pretty heavy handed!
      if (m_currentState != m_previousState)
      {
         emit controllerNewState(m_currentState);
      }

      if (!State::batteryEquals(m_previousState, m_currentState))
      {
         emit controllerNewBatteryState(m_currentState.batteryType, m_currentState.batteryLevel);
      }

      m_previousState = m_currentState;
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
   XInputSetState(m_controllerNum, &xInputVibration);
}

bool XboxController::isStateChanged(void)
{
   return m_currentState == m_previousState;
}

XboxController::~XboxController()
{
   setVibration(0, 0);

   m_pollingTimer->stop();
   delete m_pollingTimer;
}

bool operator==(const XboxController::State& a, const XboxController::State& b){
   return XboxController::State::equals(a,b);
}

bool operator!=(const XboxController::State& a, const XboxController::State& b){
   return !XboxController::State::equals(a, b);
}
