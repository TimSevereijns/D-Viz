#ifndef GAMEPADMONITOR_H
#define GAMEPADMONITOR_H

#include "../constants.h"

#include <QtGamepad/QGamepad>

/**
 * @brief The CustomGamepad class
 */
class Gamepad final : public QGamepad
{
   public:

      Gamepad(
         int deviceId = 0,
         QObject* parent = nullptr);

      bool IsLeftTriggerDown() const;

      bool IsRightTriggerDown() const;
};

#endif // GAMEPADMONITOR_H
