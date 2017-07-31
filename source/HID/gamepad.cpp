#include "gamepad.h"

CustomGamepad::CustomGamepad(
   int deviceId,
   QObject*parent)
   :
   QGamepad{ deviceId, parent }
{
}

bool CustomGamepad::IsLeftTriggerDown() const
{
   return buttonL2() >= Constants::Input::TRIGGER_ACTUATION_THRESHOLD;
}

bool CustomGamepad::IsRightTriggerDown() const
{
   return buttonR2() >= Constants::Input::TRIGGER_ACTUATION_THRESHOLD;
}
