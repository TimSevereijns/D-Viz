#ifndef CONSTANTS
#define CONSTANTS

#include <QVector3D>

#include <cmath>
#include <limits>

namespace Constants
{
   namespace FileSize
   {
      enum struct Prefix
      {
         BINARY,
         DECIMAL
      };

      namespace Binary
      {
         const static auto ONE_KIBIBYTE = static_cast<std::uintmax_t>(std::pow(2, 10));
         const static auto ONE_MEBIBYTE = static_cast<std::uintmax_t>(std::pow(2, 20));
         const static auto ONE_GIBIBYTE = static_cast<std::uintmax_t>(std::pow(2, 30));
         const static auto ONE_TEBIBYTE = static_cast<std::uintmax_t>(std::pow(2, 40));
      }

      namespace Decimal
      {
         const static auto ONE_KILOBYTE = static_cast<std::uintmax_t>(std::pow(10, 3));
         const static auto ONE_MEGABYTE = static_cast<std::uintmax_t>(std::pow(10, 6));
         const static auto ONE_GIGABYTE = static_cast<std::uintmax_t>(std::pow(10, 9));
         const static auto ONE_TERABYTE = static_cast<std::uintmax_t>(std::pow(10, 12));
      }
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
      const static QVector3D CORAL           { 1.0f,           127.0f / 255.0f,    80.0f / 255.0f };
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
