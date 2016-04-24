#include "xboxController.h"

#include <algorithm>
#include <cmath>
#include <iostream>

const uint8_t XboxController::MAX_TRIGGER_VALUE = std::numeric_limits<int8_t>::max();
const uint8_t XboxController::MIN_TRIGGER_VALUE = std::numeric_limits<int8_t>::min();

const int16_t XboxController::MAX_STICK_VALUE = std::numeric_limits<int16_t>::max();
const int16_t XboxController::MIN_STICK_VALUE = std::numeric_limits<int16_t>::min();

const int32_t XboxController::MAX_VIBRATION_VALUE = std::numeric_limits<int32_t>::max();
const int32_t XboxController::MIN_VIBRATION_VALUE = std::numeric_limits<int32_t>::min();

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
   void UpdateSingleButton(
      const unsigned int targetButton,
      std::unordered_map<unsigned int, XboxController::StateAndHandlers>& buttonMap,
      const uint16_t currentState,
      const uint16_t previousState)
   {
      auto& stateAndHandler = buttonMap[targetButton];
      const bool isButtonDown = (currentState & targetButton);
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
   void UpdateAllButtons(
      const uint16_t currentState,
      const uint16_t previousState,
      std::unordered_map<unsigned int, XboxController::StateAndHandlers>& buttonMap)
   {
      std::for_each(std::begin(buttonMap), std::end(buttonMap),
         [&buttonMap, currentState, previousState] (const auto& pair)
      {
         UpdateSingleButton(pair.first, buttonMap, currentState, previousState);
      });
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
   bool ProcessStickDeadZone(
      int rawXValue,
      int rawYValue,
      float& xValue,
      float& yValue,
      const unsigned int deadZoneRadius)
   {
      xValue = 0;
      yValue = 0;

      // Make values symetrical (otherwise negative value can be 1 unit larger than positive value):
      rawXValue = std::max<int>(rawXValue, XboxController::MIN_STICK_VALUE + 1);
      rawYValue = std::max<int>(rawYValue, XboxController::MIN_STICK_VALUE + 1);

      auto magnitude = static_cast<float>(sqrt(rawXValue * rawXValue + rawYValue * rawYValue));
      if (magnitude < deadZoneRadius)
      {
         return false;
      }

      // Remap values to make deadzone transparent:
      xValue = static_cast<float>(rawXValue) / magnitude;
      yValue = static_cast<float>(rawYValue) / magnitude;

      magnitude = std::min(magnitude, static_cast<float>(XboxController::MAX_STICK_VALUE));

      const auto normalizedMagnitude = (magnitude - deadZoneRadius) /
         (XboxController::MAX_STICK_VALUE - deadZoneRadius);

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
   bool ProcessTriggerThreshold(
      const uint8_t rawValue,
      float& value,
      const unsigned int triggerThreshold)
   {
      value = 0;
      if (rawValue < triggerThreshold)
      {
         return false;
      }

      value = (static_cast<float>(rawValue) - triggerThreshold) /
         (XboxController::MAX_TRIGGER_VALUE - triggerThreshold);

      return true;
   }
}

bool XboxController::State::operator==(const XboxController::State& rhs) const
{
   return (buttons == rhs.buttons)
      && (leftThumbX == rhs.leftThumbX)
      && (leftThumbY == rhs.leftThumbY)
      && (leftTrigger == rhs.leftTrigger)
      && (rightThumbX == rhs.rightThumbX)
      && (rightThumbY == rhs.rightThumbY)
      && (rightTrigger == rhs.rightTrigger)
      && (batteryType == rhs.batteryType)
      && (batteryLevel == rhs.batteryLevel);
}

bool XboxController::State::operator!=(const XboxController::State& rhs) const
{
   return !(*this == rhs);
}

bool XboxController::State::BatteryEquals(const State& lhs, const State& rhs)
{
   return (lhs.batteryType == rhs.batteryType) && (lhs.batteryLevel == rhs.batteryLevel);
}

XboxController::XboxController(
   unsigned int controllerNumber,
   int16_t leftStickDeadZone,
   int16_t rightStickDeadZone,
   uint8_t triggerThreshold,
   QObject* parent)
   :
   QObject{ parent },
   m_isCurrentControllerConnected{ false },
   m_isPreviousControllerConnected{ false },
   m_leftStickDeadZone{ std::min(leftStickDeadZone, XboxController::MAX_STICK_VALUE) },
   m_rightStickDeadZone{ std::min(rightStickDeadZone, XboxController::MAX_STICK_VALUE) },
   m_triggerThreshold{ std::min(triggerThreshold, XboxController::MAX_TRIGGER_VALUE) },
   m_pollingTimer{ new QTimer }
{
   m_controllerNumber = std::min(controllerNumber, 3u);
   connect(m_pollingTimer.get(), SIGNAL(timeout()), this, SLOT(Update()));
}

void XboxController::StartAutoPolling(unsigned int interval)
{
   m_pollingTimer->start(interval);
}

void XboxController::StopAutoPolling()
{
   m_pollingTimer->stop();
}

void XboxController::SetDownHandler(
   const unsigned int targetButton,
   const std::function<void ()>& handler)
{
   m_buttonMap[targetButton].onButtonDown = handler;
}

void XboxController::SetUpHandler(
   const unsigned int targetButton,
   const std::function<void ()>& handler)
{
   m_buttonMap[targetButton].onButtonUp = handler;
}

bool XboxController::IsButtonDown(const unsigned int button) const
{
   const auto result = m_buttonMap.find(button);
   if (result != std::end(m_buttonMap))
   {
      return result->second.state == KEY_STATE::DOWN;
   }

   return false;
}

void XboxController::Update()
{
   XINPUT_STATE inputState;
   memset(&inputState, 0, sizeof(XINPUT_STATE));
   m_isCurrentControllerConnected =
      (XInputGetState(m_controllerNumber, &inputState) == ERROR_SUCCESS);

   // Handle gamepad connection/deconnection:
   if (m_isPreviousControllerConnected == false && m_isCurrentControllerConnected == true)
   {
      emit ControllerConnected(m_controllerNumber);
   }
   else if (m_isPreviousControllerConnected == true && m_isCurrentControllerConnected == false)
   {
      emit ControllerDisconnected(m_controllerNumber);
   }

   m_isPreviousControllerConnected = m_isCurrentControllerConnected;

   if (!m_isCurrentControllerConnected)
   {
      return;
   }

   // Fetch the state of the buttons:
   m_currentState.buttons = inputState.Gamepad.wButtons;

   UpdateAllButtons(m_currentState.buttons, m_previousState.buttons, m_buttonMap);

   // Process stick deadzone:
   ProcessStickDeadZone(
      inputState.Gamepad.sThumbLX,
      inputState.Gamepad.sThumbLY,
      m_currentState.leftThumbX,
      m_currentState.leftThumbY,
      m_leftStickDeadZone);

   ProcessStickDeadZone(
      inputState.Gamepad.sThumbRX,
      inputState.Gamepad.sThumbRY,
      m_currentState.rightThumbX,
      m_currentState.rightThumbY,
      m_rightStickDeadZone);

   // Process trigger thresholds:
   ProcessTriggerThreshold(
      inputState.Gamepad.bLeftTrigger,
      m_currentState.leftTrigger,
      m_triggerThreshold);

   ProcessTriggerThreshold(
      inputState.Gamepad.bRightTrigger,
      m_currentState.rightTrigger,
      m_triggerThreshold);

   // Update battery states:
   XINPUT_BATTERY_INFORMATION inputBattery;
   memset(&inputBattery, 0, sizeof(XINPUT_BATTERY_INFORMATION));

   const auto batteryDataFetchResult = XInputGetBatteryInformation(
      m_controllerNumber,
      BATTERY_DEVTYPE_GAMEPAD,
      &inputBattery);

   if ( batteryDataFetchResult == ERROR_SUCCESS)
   {
      m_currentState.batteryType = inputBattery.BatteryType;
      m_currentState.batteryLevel = inputBattery.BatteryLevel;
   }
   else
   {
      m_currentState.batteryType = BATTERY_TYPE_UNKNOWN;
      m_currentState.batteryLevel = BATTERY_LEVEL_EMPTY;
   }

   if (m_currentState != m_previousState)
   {
      emit NewControllerState(m_currentState);
   }

   if (!State::BatteryEquals(m_previousState, m_currentState))
   {
      emit NewControllerBatteryState(m_currentState.batteryType, m_currentState.batteryLevel);
   }

   m_previousState = m_currentState;
}

void XboxController::SetLeftStickDeadZone(int16_t newDeadZone)
{
   m_leftStickDeadZone = std::min(newDeadZone, XboxController::MAX_STICK_VALUE);
}

void XboxController::SetRightStickDeadZone(int16_t newDeadZone)
{
   m_rightStickDeadZone = std::min(newDeadZone, XboxController::MAX_STICK_VALUE);
}

void XboxController::SetTriggerThreshold(uint8_t newThreshold)
{
   m_triggerThreshold = std::min(newThreshold, XboxController::MAX_TRIGGER_VALUE);
}

void XboxController::SetVibration(const float leftVibration, const float rightVibration)
{
   XINPUT_VIBRATION xInputVibration;
   xInputVibration.wLeftMotorSpeed = MAX_VIBRATION_VALUE * qBound(0.0f, 1.0f, leftVibration);
   xInputVibration.wRightMotorSpeed = MAX_VIBRATION_VALUE * qBound(0.0f, 1.0f, rightVibration);
   XInputSetState(m_controllerNumber, &xInputVibration);
}

bool XboxController::HasStateChanged() const
{
   return m_currentState == m_previousState;
}

bool XboxController::IsConnected() const
{
   return m_isCurrentControllerConnected;
}

const XboxController::State& XboxController::GetCurrentState() const
{
   return m_currentState;
}

XboxController::~XboxController()
{
   SetVibration(/*leftVibration =*/ 0, /*rightVibration =*/0);

   m_pollingTimer->stop();
}
