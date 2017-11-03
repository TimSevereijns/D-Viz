#ifndef CONSTANTS
#define CONSTANTS

#include <QVector3D>

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
      constexpr static auto THREAD_LIMIT{ 4u };
   }

   namespace Logging
   {
      constexpr static auto& DEFAULT_LOG{ "D-Viz" };
   }
}

#endif // CONSTANTS
