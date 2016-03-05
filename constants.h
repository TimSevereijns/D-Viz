#ifndef CONSTANTS
#define CONSTANTS

#include <QVector3D>

namespace Constants
{
   // @todo UPGRADE TO C++11: constexpr instead of const

   namespace FileSizeUnits
   {
      const static double oneKibibyte = std::pow(2, 10);
      const static double oneMebibyte = std::pow(2, 20);
      const static double oneGibibyte = std::pow(2, 30);
      const static double oneTebibyte = std::pow(2, 40);
   }

   namespace Colors
   {                                          // Red,             Green,           Blue
      const static QVector3D canaryYellow       {1.0f,            239.0f / 255.0f, 0.0f};
      const static QVector3D hotPink            {1.0f,            105.0f / 255.0f, 180.0f / 255.0f};
      const static QVector3D metallicSeaweed    {8.0f / 255.0f,   0.0f,            126.0f / 255.0f};
   }
}

#endif // CONSTANTS

