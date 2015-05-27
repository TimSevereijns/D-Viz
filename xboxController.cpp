#include "xboxController.h"

#include <cmath>
#include <iostream>

namespace
{
   /**
    * @brief UpdateSingleButton
    *
    * @param targetButton
    * @param buttonMap
    * @param currentState
    * @param previousState
    */
   void UpdateSingleButton(const unsigned int targetButton,
      std::map<unsigned int, XboxController::StateAndHandlers>& buttonMap,
      const quint16 currentState, const quint16 previousState)
   {
      auto& stateAndHandler = buttonMap[targetButton];
      const auto isButtonDown = (currentState & targetButton);
      if (isButtonDown && !(previousState & targetButton))
      {
         stateAndHandler.state = XboxController::KEY_STATE::DOWN;
         if (stateAndHandler.onButtonDown)
         {
            stateAndHandler.onButtonDown();
         }
      }
      else if (!isButtonDown && (previousState & targetButton))
      {
         stateAndHandler.state = XboxController::KEY_STATE::UP;
         if (stateAndHandler.onButtonUp)
         {
            stateAndHandler.onButtonUp();
         }
      }
   }

   /**
    * @brief UpdateAllButtons
    *
    * @param currentState
    * @param previousState
    * @param buttonMap
    */
   void UpdateAllButtons(const quint16 currentState, const quint16 previousState,
      std::map<unsigned int, XboxController::StateAndHandlers>& buttonMap)
   {
      UpdateSingleButton(XINPUT_GAMEPAD_A, buttonMap, currentState, previousState);
      UpdateSingleButton(XINPUT_GAMEPAD_B, buttonMap, currentState, previousState);
      UpdateSingleButton(XINPUT_GAMEPAD_X, buttonMap, currentState, previousState);
      UpdateSingleButton(XINPUT_GAMEPAD_Y, buttonMap, currentState, previousState);
      UpdateSingleButton(XINPUT_GAMEPAD_LEFT_SHOULDER, buttonMap, currentState, previousState);
      UpdateSingleButton(XINPUT_GAMEPAD_RIGHT_SHOULDER, buttonMap, currentState, previousState);
      UpdateSingleButton(XINPUT_GAMEPAD_LEFT_THUMB, buttonMap, currentState, previousState);
      UpdateSingleButton(XINPUT_GAMEPAD_RIGHT_THUMB, buttonMap, currentState, previousState);
      UpdateSingleButton(XINPUT_GAMEPAD_BACK, buttonMap, currentState, previousState);
      UpdateSingleButton(XINPUT_GAMEPAD_START, buttonMap, currentState, previousState);
      UpdateSingleButton(XINPUT_GAMEPAD_DPAD_UP, buttonMap, currentState, previousState);
      UpdateSingleButton(XINPUT_GAMEPAD_DPAD_LEFT, buttonMap, currentState, previousState);
      UpdateSingleButton(XINPUT_GAMEPAD_DPAD_DOWN, buttonMap, currentState, previousState);
      UpdateSingleButton(XINPUT_GAMEPAD_DPAD_RIGHT, buttonMap, currentState, previousState);
   }

   /**
    * @brief XboxController::processStickDeadZone
    *
    * @param rawXValue
    * @param rawYValue
    * @param xValue
    * @param yValue
    * @param deadZoneRadius
    * @return
    */
   bool ProcessStickDeadZone(qint16 rawXValue, qint16 rawYValue, float& xValue, float& yValue,
      const unsigned int deadZoneRadius)
   {
      xValue = 0;
      yValue = 0;

      // Make values symetrical (otherwise negative value can be 1 unit larger than positive value):
      rawXValue = qMax(rawXValue, (qint16) - MAX_STICK_VALUE);
      rawYValue = qMax(rawYValue, (qint16) - MAX_STICK_VALUE);

      float magnitude = sqrt(rawXValue * rawXValue + rawYValue * rawYValue);
      if (magnitude < deadZoneRadius)
      {
         return false;
      }

      // Remap values to make deadzone transparent:
      xValue = ((float) rawXValue) / magnitude;
      yValue = ((float) rawYValue) / magnitude;

      magnitude = qMin(magnitude, (float) MAX_STICK_VALUE);
      float normalizedMagnitude = (magnitude - deadZoneRadius) / ((MAX_STICK_VALUE - deadZoneRadius));

      xValue *= normalizedMagnitude;
      yValue *= normalizedMagnitude;

      return true;
   }

   /**
    * @brief XboxController::processTriggerThreshold
    *
    * @param rawValue
    * @param value
    * @param triggerThreshold
    * @return
    */
   bool ProcessTriggerThreshold(const quint8 rawValue, float& value,
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
      })
{
}

bool XboxController::State::equals(const State& lhs, const State& rhs)
{
   return (lhs.buttons == rhs.buttons)
      && (lhs.leftThumbX == rhs.leftThumbX)
      && (lhs.leftThumbY == rhs.leftThumbY)
      && (lhs.leftTrigger == rhs.leftTrigger)
      && (lhs.rightThumbX == rhs.rightThumbX)
      && (lhs.rightThumbY == rhs.rightThumbY)
      && (lhs.rightTrigger == rhs.rightTrigger)
      && (lhs.batteryType == rhs.batteryType)
      && (lhs.batteryLevel == rhs.batteryLevel);
}

bool XboxController::State::batteryEquals(const State& lhs, const State& rhs)
{
   return (lhs.batteryType == rhs.batteryType) && (lhs.batteryLevel == rhs.batteryLevel);
}

bool XboxController::State::IsButtonDown(const unsigned int button) const
{
   const auto result = m_buttonMap.find(button);
   if (result != std::end(m_buttonMap))
   {
      return result->second.state == KEY_STATE::DOWN;
   }

   return false;
}

XboxController::XboxController(unsigned int controllerNum, unsigned int leftStickDeadZone,
   unsigned int rightStickDeadZone, unsigned int triggerThreshold, QObject* parent)
   : QObject(parent),
     m_isCurrentControllerConnected(false),
     m_isPreviousControllerConnected(false),
     m_leftStickDeadZone(qMin(leftStickDeadZone, MAX_STICK_VALUE)),
     m_rightStickDeadZone(qMin(rightStickDeadZone, MAX_STICK_VALUE)),
     m_triggerThreshold(qMin(triggerThreshold, MAX_TRIGGER_VALUE))
{
   m_controllerNum = qMin(controllerNum, 3u);
   m_pollingTimer = new QTimer();
   connect(m_pollingTimer, SIGNAL(timeout()), this, SLOT(Update()));
}

void XboxController::StartAutoPolling(unsigned int interval)
{
   m_pollingTimer->start(interval);
}

void XboxController::StopAutoPolling()
{
   m_pollingTimer->stop();
}

void XboxController::SetHandler(const unsigned int targetButton,
   const XboxController::KEY_STATE targetState, const std::function<void ()>& handler)
{
   if (targetState == XboxController::KEY_STATE::UP)
   {
      m_currentState.m_buttonMap[targetButton].onButtonUp = handler;
   }
   else if (targetState == XboxController::KEY_STATE::DOWN)
   {
      m_currentState.m_buttonMap[targetButton].onButtonDown = handler;
   }
}

void XboxController::Update()
{
   XINPUT_STATE xInputState;
   memset(&xInputState, 0, sizeof(XINPUT_STATE));
   m_isCurrentControllerConnected = (XInputGetState(m_controllerNum, &xInputState) == ERROR_SUCCESS);

   //Handling gamepad connection/deconnection:
   if (m_isPreviousControllerConnected == false && m_isCurrentControllerConnected == true)
   {
      emit ControllerConnected(m_controllerNum);
   }
   else if (m_isPreviousControllerConnected == true && m_isCurrentControllerConnected == false)
   {
      emit ControllerDisconnected(m_controllerNum);
   }

   m_isPreviousControllerConnected = m_isCurrentControllerConnected;

   if (m_isCurrentControllerConnected)
   {
      // Fetch the state of the buttons:
      m_currentState.buttons = xInputState.Gamepad.wButtons;

      UpdateAllButtons(m_currentState.buttons, m_previousState.buttons,
         m_currentState.m_buttonMap);

      // Process stick deadzone:
      ProcessStickDeadZone(xInputState.Gamepad.sThumbLX, xInputState.Gamepad.sThumbLY,
         m_currentState.leftThumbX, m_currentState.leftThumbY, m_leftStickDeadZone);
      ProcessStickDeadZone(xInputState.Gamepad.sThumbRX, xInputState.Gamepad.sThumbRY,
         m_currentState.rightThumbX, m_currentState.rightThumbY, m_rightStickDeadZone);

      // Process trigger thresholds:
      ProcessTriggerThreshold(xInputState.Gamepad.bLeftTrigger, m_currentState.leftTrigger,
         m_triggerThreshold);
      ProcessTriggerThreshold(xInputState.Gamepad.bRightTrigger, m_currentState.rightTrigger,
         m_triggerThreshold);

      // Update battery states:
      XINPUT_BATTERY_INFORMATION xInputBattery;
      memset(&xInputBattery,0,sizeof(XINPUT_BATTERY_INFORMATION));
      const auto batteryDataFetchResult = XInputGetBatteryInformation(m_controllerNum,
         BATTERY_DEVTYPE_GAMEPAD, &xInputBattery);
      if ( batteryDataFetchResult == ERROR_SUCCESS)
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
         emit NewControllerState(m_currentState);
      }

      if (!State::batteryEquals(m_previousState, m_currentState))
      {
         emit NewControllerBatteryState(m_currentState.batteryType, m_currentState.batteryLevel);
      }

      m_previousState = m_currentState;
   }
}

void XboxController::SetLeftStickDeadZone(unsigned int newDeadZone)
{
   m_leftStickDeadZone = qMin(newDeadZone, MAX_STICK_VALUE);
}

void XboxController::SetRightStickDeadZone(unsigned int newDeadZone)
{
   m_rightStickDeadZone = qMin(newDeadZone, MAX_STICK_VALUE);
}

void XboxController::SetTriggerThreshold(unsigned int newThreshold)
{
   m_triggerThreshold = qMin(newThreshold, MAX_TRIGGER_VALUE);
}

void XboxController::SetVibration(const float leftVibration, const float rightVibration)
{
   XINPUT_VIBRATION xInputVibration;
   xInputVibration.wLeftMotorSpeed = MAX_VIBRATION_VALUE * qBound(0.0f, 1.0f, leftVibration);
   xInputVibration.wRightMotorSpeed = MAX_VIBRATION_VALUE * qBound(0.0f, 1.0f, rightVibration);
   XInputSetState(m_controllerNum, &xInputVibration);
}

bool XboxController::HasStateChanged(void)
{
   return m_currentState == m_previousState;
}

bool XboxController::IsConnected()
{
   return m_isCurrentControllerConnected;
}

const XboxController::State& XboxController::GetCurrentState()
{
   return m_currentState;
}

XboxController::~XboxController()
{
   SetVibration(0, 0);

   m_pollingTimer->stop();
   delete m_pollingTimer;
}

bool operator==(const XboxController::State& a, const XboxController::State& b){
   return XboxController::State::equals(a,b);
}

bool operator!=(const XboxController::State& a, const XboxController::State& b){
   return !XboxController::State::equals(a, b);
}
