#ifndef CONSTANTS
#define CONSTANTS

#include <QVector3D>

#include "literals.h"

#ifdef Q_OS_WIN
   #undef RGB
#endif // Q_OS_WIN

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
      constexpr static auto GREEN         = RGB(  0,  255,    0);
      constexpr static auto BLUE          = RGB(  0,    0,  255);
      constexpr static auto BABY_BLUE     = RGB(137,  207,  240);
      constexpr static auto CANARY_YELLOW = RGB(255,  239,    0);
      constexpr static auto HOT_PINK      = RGB(255,  105,  180);
      constexpr static auto FILE_GREEN    = RGB(128,  255,  128);
      constexpr static auto SLATE_GRAY    = RGB(112,  128,  144);
      constexpr static auto WHITE         = RGB(255,  255,  255);
      constexpr static auto CORAL         = RGB(255,  127,   80);
   }

   namespace ColorScheme
   {
      constexpr static auto& DEFAULT{ L"Default" };
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
      static std::string DEFAULT_LOG{ "D-Viz" };
      static std::string FILESYSTEM_LOG{ "Filesystem" };
   }

   namespace Math
   {
      constexpr static auto PI = 3.14159265358979323846;
      constexpr static auto RADIANS_TO_DEGREES = 180.0 / PI;
      constexpr static auto DEGREES_TO_RADIANS = PI / 180.0;
   }

   namespace Preferences
   {
      constexpr static auto& SHOW_ORIGIN{ L"showOrigin" };
      constexpr static auto& SHOW_GRID{ L"showGrid" };
      constexpr static auto& SHOW_LIGHT_MARKERS{ L"showLightMarkers" };
      constexpr static auto& SHOW_FRUSTUM { L"showFrustum" };
   }

   namespace Visualization
   {
      constexpr static const double PADDING_RATIO{ 0.9 };
      constexpr static const double MAX_PADDING{ 0.75 };

      constexpr static const float BLOCK_HEIGHT{ 2.0f };
      constexpr static const float ROOT_BLOCK_WIDTH{ 1000.0f };
      constexpr static const float ROOT_BLOCK_DEPTH{ 1000.0f };
   }
}

#endif // CONSTANTS
