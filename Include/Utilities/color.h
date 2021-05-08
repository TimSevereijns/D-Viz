#ifndef COLOR_H
#define COLOR_H

#include <QVector3D>

namespace Color
{
    constexpr QVector3D FromRGB(int red, int green, int blue) noexcept
    {
        return { red / 255.0f, green / 255.0f, blue / 255.0f };
    }
} // namespace Color

#endif // COLOR_H
