#include "xboxController.h"

#include <algorithm>
#include <cmath>

const uint8_t XboxController::MAX_TRIGGER_VALUE = std::numeric_limits<int8_t>::max();
const uint8_t XboxController::MIN_TRIGGER_VALUE = std::numeric_limits<int8_t>::min();

const int16_t XboxController::MAX_STICK_VALUE = std::numeric_limits<int16_t>::max();
const int16_t XboxController::MIN_STICK_VALUE = std::numeric_limits<int16_t>::min();

const int32_t XboxController::MAX_VIBRATION_VALUE = std::numeric_limits<int32_t>::max();
const int32_t XboxController::MIN_VIBRATION_VALUE = std::numeric_limits<int32_t>::min();

namespace
{
   /**
    * @brief Updates the state of a single controller button.
    *
    * @param[in] targetButton       The specific button to be targeted for a state update.
    * @param[in] buttonMap          The button map containing the button state to be updated.
    * @param[in] currentState       The current state of the controller buttons.
    * @param[in] previousState      The previous state of the controller buttons.
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
    * @brief Extracts the state of all the buttons from the passed in button state mask.
    *
    * @param[in] currentState       The current state of the controller buttons.
    * @param[in] previousState      The previous state of the controller buttons.
    * @param[in, out] buttonMap     The button map to be updated if any of the buttons have changed
    *                               state.
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
    * @param[in] rawXValue          The raw x-value input to be processed.
    * @param[in] rawYValue          The raw y-value inpput to be processed.
    * @param[out] xValue            The new x-value, assuming the raw input exceeds the dead zone.
    * @param[out] yValue            The new y-value, assiming the raw input exceeds the dead zone.
    * @param[in] deadZoneRadius     The dead zone radius that must be exceeded before a new value is
    *                               considered valid.
    *
    * @returns True if the processed thumbstick value exceeds the dead zone radius; false otherwise.
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
    * @param[in] rawValue           Raw trigger input value.
    * @param[out] value             The new trigger value, assuming the threshold is met.
    * @param[in] triggerThreshold   The threshold value to be met.
    *
    * @returns True if the trigger threshold was met.
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
      && (rightTrigger == rhs.rightTrigger);
}

bool XboxController::State::operator!=(const XboxController::State& rhs) const
{
   return !(*this == rhs);
}

XboxController::XboxController(
   int controllerNumber,
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
   m_controllerNumber = std::min(controllerNumber, XUSER_MAX_COUNT - 1);
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
   ZeroMemory(&inputState, sizeof(XINPUT_STATE));

   const auto stateRetrievalResult = XInputGetState(m_controllerNumber, &inputState);
   m_isCurrentControllerConnected = (stateRetrievalResult == ERROR_SUCCESS);

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

   if (m_currentState != m_previousState)
   {
      emit NewControllerState(m_currentState);
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
