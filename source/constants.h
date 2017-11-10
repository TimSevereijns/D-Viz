#ifndef CONSTANTS
#define CONSTANTS

#include <QVector3D>

#include "literals.h"

#ifdef Q_OS_WIN
   #undef RGB
#endif

namespace
{
   constexpr QVector3D RGB(int red, int green, int blue) noexcept
   {
      return { red / 255.0f, green / 255.0f, blue / 255.0f };
   }
}

namespace Constants
{
   namespace FileSize
   {
      enum struct Prefix
      {
         BINARY,
         DECIMAL
      };
   }

   namespace Colors
   {
      constexpr static auto RED           = RGB(255,    0,    0);
      constexpr static auto GREEN         = RGB(  0,    1,    0);
      constexpr static auto BLUE          = RGB(  0,    0,    1);
      constexpr static auto CANARY_YELLOW = RGB(255,  239,    0);
      constexpr static auto HOT_PINK      = RGB(255,  105,  180);
      constexpr static auto FILE_GREEN    = RGB(128,  255,  128);
      constexpr static auto WHITE         = RGB(255,  255,  255);
   }

   namespace Graphics
   {
      constexpr static auto DESIRED_TIME_BETWEEN_FRAMES{ 20 };
   }

   namespace Input
   {
      constexpr static auto MOVEMENT_AMPLIFICATION{ 10.0 };
      constexpr static auto TRIGGER_ACTUATION_THRESHOLD{ 0.2 };
   }

   namespace Concurrency
   {
      constexpr static auto THREAD_LIMIT{ 4u };
   }

   namespace Logging
   {
      constexpr static auto& DEFAULT_LOG{ "D-Viz" };
   }
}

#endif // CONSTANTS
