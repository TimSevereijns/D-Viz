#include "View/HID/gamepad.h"

Gamepad::Gamepad(int deviceId, QObject* parent) : QGamepad{ deviceId, parent }
{
}

bool Gamepad::IsLeftTriggerDown() const
{
    return buttonL2() >= Constants::Input::TriggerActuationThreshold;
}

bool Gamepad::IsRightTriggerDown() const
{
    return buttonR2() >= Constants::Input::TriggerActuationThreshold;
}
