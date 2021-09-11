#include "View/HID/xboxGamepad.h"

#include <algorithm>
#include <cmath>

namespace
{
    void UpdateSingleButton(
        unsigned int targetButton,
        std::unordered_map<unsigned int, XboxGamepad::StateAndHandlers>& buttonMap,
        std::uint16_t currentState, std::uint16_t previousState)
    {
        auto& stateAndHandler = buttonMap[targetButton];

        const bool isButtonDown = (currentState & targetButton);
        if (isButtonDown && !(previousState & targetButton)) {
            stateAndHandler.state = XboxGamepad::KeyState::Down;
            if (stateAndHandler.onButtonDown) {
                stateAndHandler.onButtonDown();
            }
        } else if (!isButtonDown && (previousState & targetButton)) {
            stateAndHandler.state = XboxGamepad::KeyState::Up;
            if (stateAndHandler.onButtonUp) {
                stateAndHandler.onButtonUp();
            }
        }
    }

    void UpdateAllButtons(
        std::uint16_t currentState, std::uint16_t previousState,
        std::unordered_map<unsigned int, XboxGamepad::StateAndHandlers>& buttonMap)
    {
        for (const auto& pair : buttonMap) {
            UpdateSingleButton(pair.first, buttonMap, currentState, previousState);
        }
    }

    bool ProcessStickDeadZone(
        int rawXValue, int rawYValue, float& xValue, float& yValue,
        const unsigned int deadZoneRadius)
    {
        xValue = 0.0f;
        yValue = 0.0f;

        // Make values symetrical (otherwise negative value can be 1 unit larger than positive
        // value):
        rawXValue = std::max<int>(rawXValue, XboxGamepad::MinStickValue + 1);
        rawYValue = std::max<int>(rawYValue, XboxGamepad::MinStickValue + 1);

        auto magnitude = static_cast<float>(sqrt(rawXValue * rawXValue + rawYValue * rawYValue));
        if (magnitude < deadZoneRadius) {
            return false;
        }

        // Remap values to make deadzone transparent:
        xValue = static_cast<float>(rawXValue) / magnitude;
        yValue = static_cast<float>(rawYValue) / magnitude;

        magnitude = std::min<float>(magnitude, XboxGamepad::MaxStickValue);

        const auto normalizedMagnitude =
            (magnitude - deadZoneRadius) / (XboxGamepad::MaxStickValue - deadZoneRadius);

        xValue *= normalizedMagnitude;
        yValue *= normalizedMagnitude;

        return true;
    }

    bool ProcessTriggerThreshold(std::uint8_t rawValue, float& value, unsigned int triggerThreshold)
    {
        value = 0.0f;
        if (rawValue < triggerThreshold) {
            return false;
        }

        value = (static_cast<float>(rawValue) - triggerThreshold) /
                (XboxGamepad::MaxTriggerValue - triggerThreshold);

        return true;
    }
} // namespace

bool XboxGamepad::State::operator==(const XboxGamepad::State& rhs) const
{
    return (buttons == rhs.buttons) && (leftThumbX == rhs.leftThumbX) &&
           (leftThumbY == rhs.leftThumbY) && (leftTrigger == rhs.leftTrigger) &&
           (rightThumbX == rhs.rightThumbX) && (rightThumbY == rhs.rightThumbY) &&
           (rightTrigger == rhs.rightTrigger);
}

bool XboxGamepad::State::operator!=(const XboxGamepad::State& rhs) const
{
    return !(*this == rhs);
}

XboxGamepad::XboxGamepad(int controllerNumber, QObject* parent)
    : QObject{ parent }, m_pollingTimer{ new QTimer }
{
    m_controllerNumber = std::min(controllerNumber, XUSER_MAX_COUNT - 1);
    connect(m_pollingTimer.get(), SIGNAL(timeout()), this, SLOT(Update()));
}

void XboxGamepad::StartAutoPolling(unsigned int interval)
{
    m_pollingTimer->start(interval);
}

void XboxGamepad::StopAutoPolling()
{
    m_pollingTimer->stop();
}

void XboxGamepad::SetDownHandler(unsigned int targetButton, const std::function<void()>& handler)
{
    m_buttonMap[targetButton].onButtonDown = handler;
}

void XboxGamepad::SetUpHandler(unsigned int targetButton, const std::function<void()>& handler)
{
    m_buttonMap[targetButton].onButtonUp = handler;
}

bool XboxGamepad::IsButtonDown(unsigned int button) const
{
    const auto result = m_buttonMap.find(button);
    if (result != std::end(m_buttonMap)) {
        return result->second.state == KeyState::Down;
    }

    return false;
}

void XboxGamepad::Update()
{
    XINPUT_STATE inputState;
    ZeroMemory(&inputState, sizeof(XINPUT_STATE));

    const auto stateRetrievalResult = XInputGetState(m_controllerNumber, &inputState);
    m_isCurrentControllerConnected = (stateRetrievalResult == ERROR_SUCCESS);

    // Handle gamepad connection/deconnection:
    if (m_isPreviousControllerConnected == false && m_isCurrentControllerConnected == true) {
        emit ControllerConnected(m_controllerNumber);
    } else if (m_isPreviousControllerConnected == true && m_isCurrentControllerConnected == false) {
        emit ControllerDisconnected(m_controllerNumber);
    }

    m_isPreviousControllerConnected = m_isCurrentControllerConnected;

    if (!m_isCurrentControllerConnected) {
        return;
    }

    // Fetch the state of the buttons:
    m_currentState.buttons = inputState.Gamepad.wButtons;

    UpdateAllButtons(m_currentState.buttons, m_previousState.buttons, m_buttonMap);

    // Process stick deadzone:
    ProcessStickDeadZone(
        inputState.Gamepad.sThumbLX, inputState.Gamepad.sThumbLY, m_currentState.leftThumbX,
        m_currentState.leftThumbY, m_leftStickDeadZone);

    ProcessStickDeadZone(
        inputState.Gamepad.sThumbRX, inputState.Gamepad.sThumbRY, m_currentState.rightThumbX,
        m_currentState.rightThumbY, m_rightStickDeadZone);

    // Process trigger thresholds:
    ProcessTriggerThreshold(
        inputState.Gamepad.bLeftTrigger, m_currentState.leftTrigger, m_triggerThreshold);

    ProcessTriggerThreshold(
        inputState.Gamepad.bRightTrigger, m_currentState.rightTrigger, m_triggerThreshold);

    if (m_currentState != m_previousState) {
        emit NewControllerState(m_currentState);
    }

    m_previousState = m_currentState;
}

void XboxGamepad::SetLeftStickDeadZone(std::int16_t newDeadZone)
{
    m_leftStickDeadZone = std::min(newDeadZone, XboxGamepad::MaxStickValue);
}

void XboxGamepad::SetRightStickDeadZone(std::int16_t newDeadZone)
{
    m_rightStickDeadZone = std::min(newDeadZone, XboxGamepad::MaxStickValue);
}

void XboxGamepad::SetTriggerThreshold(std::uint8_t newThreshold)
{
    m_triggerThreshold = std::min(newThreshold, XboxGamepad::MaxTriggerValue);
}

void XboxGamepad::SetVibration(float leftVibration, float rightVibration)
{
    XINPUT_VIBRATION xInputVibration;

    xInputVibration.wLeftMotorSpeed =
        static_cast<float>(MaxVibrationValue) * std::clamp(leftVibration, 0.0f, 1.0f);

    xInputVibration.wRightMotorSpeed =
        static_cast<float>(MaxVibrationValue) * std::clamp(rightVibration, 0.0f, 1.0f);

    XInputSetState(m_controllerNumber, &xInputVibration);
}

bool XboxGamepad::HasStateChanged() const
{
    return m_currentState == m_previousState;
}

bool XboxGamepad::IsConnected() const
{
    return m_isCurrentControllerConnected;
}

const XboxGamepad::State& XboxGamepad::GetCurrentState() const
{
    return m_currentState;
}

XboxGamepad::~XboxGamepad()
{
    constexpr auto vibrationAmount = 0;
    SetVibration(vibrationAmount, vibrationAmount);

    m_pollingTimer->stop();
}
