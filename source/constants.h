#ifndef CONSTANTS
#define CONSTANTS

#include <QVector3D>

#include <limits>

namespace Constants
{
   namespace FileSize
   {
      const static auto ONE_KIBIBYTE = static_cast<std::uintmax_t>(std::pow(2, 10));
      const static auto ONE_MEBIBYTE = static_cast<std::uintmax_t>(std::pow(2, 20));
      const static auto ONE_GIBIBYTE = static_cast<std::uintmax_t>(std::pow(2, 30));
      const static auto ONE_TEBIBYTE = static_cast<std::uintmax_t>(std::pow(2, 40));
   }

   namespace Colors
   {                                         // Red,                     Green,              Blue
      const static QVector3D RED             { 1.0f,                      0.0f,              0.0f};
      const static QVector3D GREEN           { 0.0f,                      1.0f,              0.0f};
      const static QVector3D BLUE            { 0.0f,                      0.0f,              1.0f};
      const static QVector3D CANARY_YELLOW   { 1.0f,           239.0f / 255.0f,             0.0f };
      const static QVector3D HOT_PINK        { 1.0f,           105.0f / 255.0f,  180.0f / 255.0f };
      const static QVector3D FILE_GREEN      { 0.5f,                      1.0f,             0.5f };
      const static QVector3D WHITE           { 1.0f,                      1.0f,             1.0f };
   }

   namespace Graphics
   {
      constexpr static auto DESIRED_TIME_BETWEEN_FRAMES{ 20 };
   }

   namespace Xbox
   {
      constexpr static auto MOVEMENT_AMPLIFICATION{ 10.0 };
      constexpr static auto TRIGGER_ACTUATION_THRESHOLD{ 0.2f };
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

#endif // CONSTANTS

