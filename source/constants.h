#ifndef CONSTANTS
#define CONSTANTS

#include <QVector3D>

#include <limits>

namespace Constants
{
   namespace FileSize
   {
      const static auto oneKibibyte = static_cast<std::uintmax_t>(std::pow(2, 10));
      const static auto oneMebibyte = static_cast<std::uintmax_t>(std::pow(2, 20));
      const static auto oneGibibyte = static_cast<std::uintmax_t>(std::pow(2, 30));
      const static auto oneTebibyte = static_cast<std::uintmax_t>(std::pow(2, 40));
   }

   namespace Colors
   {                                          // Red,             Green,           Blue
      const static QVector3D canaryYellow    { 1.0f,            239.0f / 255.0f,  0.0f };
      const static QVector3D hotPink         { 1.0f,            105.0f / 255.0f,  180.0f / 255.0f };
      const static QVector3D metallicSeaweed { 8.0f / 255.0f,   0.0f,             126.0f / 255.0f };
   }

   constexpr static int DESIRED_TIME_BETWEEN_FRAMES = 20;

   constexpr static double MOVEMENT_AMPLIFICATION = 10.0;

   constexpr static float XBOX_TRIGGER_ACTUATION_THRESHOLD = 0.2f;
}

#endif // CONSTANTS

