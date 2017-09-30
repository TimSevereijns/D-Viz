#include "gamepad.h"

Gamepad::Gamepad(
   int deviceId,
   QObject* parent)
   :
   QGamepad{ deviceId, parent }
{
}

bool Gamepad::IsLeftTriggerDown() const
{
   return buttonL2() >= Constants::Input::TRIGGER_ACTUATION_THRESHOLD;
}

bool Gamepad::IsRightTriggerDown() const
{
   return buttonR2() >= Constants::Input::TRIGGER_ACTUATION_THRESHOLD;
}
