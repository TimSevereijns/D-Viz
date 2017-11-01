#ifndef CONSTANTS
#define CONSTANTS

#include <QVector3D>

#include <cmath>
#include <limits>

namespace Literals
{
   namespace Numeric
   {
      namespace Binary
      {
         constexpr auto operator""_KiB(unsigned long long value) noexcept -> std::size_t
         {
            return value * 1'024;
         }

         constexpr auto operator""_MiB(unsigned long long value) noexcept
         {
            return value * 1'024 * 1_KiB;
         }

         constexpr auto operator""_GiB(unsigned long long value) noexcept
         {
            return value * 1'024 * 1_MiB;
         }

         constexpr auto operator""_TiB(unsigned long long value) noexcept
         {
            return value * 1'024 * 1_GiB;
         }
      }

      namespace Decimal
      {
         constexpr auto operator""_KB(unsigned long long value) noexcept -> std::size_t
         {
            return value * 1'000;
         }

         constexpr auto operator""_MB(unsigned long long value) noexcept
         {
            return value * 1'000 * 1_KB;
         }

         constexpr auto operator""_GB(unsigned long long value) noexcept
         {
            return value * 1'000 * 1_MB;
         }

         constexpr auto operator""_TB(unsigned long long value) noexcept
         {
            return value * 1'000 * 1_GB;
         }
      }
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
   {                                         // Red,                     Green,              Blue
      const static QVector3D RED             { 1.0f,                      0.0f,              0.0f };
      const static QVector3D GREEN           { 0.0f,                      1.0f,              0.0f };
      const static QVector3D BLUE            { 0.0f,                      0.0f,              1.0f };
      const static QVector3D CANARY_YELLOW   { 1.0f,           239.0f / 255.0f,             0.0f  };
      const static QVector3D HOT_PINK        { 1.0f,           105.0f / 255.0f,  180.0f / 255.0f  };
      const static QVector3D FILE_GREEN      { 0.5f,                      1.0f,             0.5f  };
      const static QVector3D WHITE           { 1.0f,                      1.0f,             1.0f  };
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
      constexpr static unsigned int THREAD_LIMIT{ 4 };
   }

   namespace Logging
   {
      const static auto& LOG_NAME{ "D-Viz" };
   }
}

// @todo Find a better way of handling this. Perhaps a Globals:: namespace...
extern Constants::FileSize::Prefix ActivePrefix;

#endif // CONSTANTS
