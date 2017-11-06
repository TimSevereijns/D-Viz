#ifndef CONSTANTS
#define CONSTANTS

#include <QVector3D>

#include "literals.h"

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
      using namespace Literals::Colors;

      constexpr static QVector3D RED           { 255_R, 000_G, 000_B };
      constexpr static QVector3D GREEN         { 000_R, 001_G, 000_B };
      constexpr static QVector3D BLUE          { 000_R, 000_G, 001_B };
      constexpr static QVector3D CANARY_YELLOW { 255_R, 239_G, 000_B };
      constexpr static QVector3D HOT_PINK      { 255_R, 105_G, 180_B };
      constexpr static QVector3D FILE_GREEN    { 128_R, 255_G, 128_B };
      constexpr static QVector3D WHITE         { 255_R, 255_G, 255_B };
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
