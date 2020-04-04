#ifndef CONSTANTS
#define CONSTANTS

#include <QVector3D>

#include "literals.h"

#ifdef Q_OS_WIN
#undef RGB
#endif // Q_OS_WIN

namespace Detail
{
    constexpr QVector3D RGB(int red, int green, int blue) noexcept
    {
        return { red / 255.0f, green / 255.0f, blue / 255.0f };
    }
} // namespace Detail

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
        constexpr auto Red = Detail::RGB(255, 0, 0);
        constexpr auto Green = Detail::RGB(0, 255, 0);
        constexpr auto Blue = Detail::RGB(0, 0, 255);
        constexpr auto BabyBlue = Detail::RGB(137, 207, 240);
        constexpr auto CanaryYellow = Detail::RGB(255, 239, 0);
        constexpr auto HotPink = Detail::RGB(255, 105, 180);
        constexpr auto FileGreen = Detail::RGB(128, 255, 128);
        constexpr auto SlateGray = Detail::RGB(112, 128, 144);
        constexpr auto White = Detail::RGB(255, 255, 255);
        constexpr auto Coral = Detail::RGB(255, 127, 80);
    } // namespace Colors

    namespace ColorScheme
    {
        constexpr auto& Default = L"Default";
    }

    namespace Graphics
    {
        constexpr auto DesiredTimeBetweenFrames = 20;
    }

    namespace Input
    {
        constexpr auto MovementAmplification = 10.0;
        constexpr auto TriggerActuationThreshold = 0.2;
    } // namespace Input

    namespace Concurrency
    {
        constexpr auto ThreadLimit = 4u;
    }

    namespace Logging
    {
        constexpr auto& DefaultLog = "D-Viz";
        constexpr auto& FilesystemLog = "Filesystem";
    } // namespace Logging

    namespace Math
    {
        constexpr auto Pi = 3.14159265358979323846;
        constexpr auto RadiansToDegrees = 180.0 / Pi;
        constexpr auto DegreesToRadians = Pi / 180.0;
    } // namespace Math

    namespace Preferences
    {
        constexpr auto& ShowOrigin = L"showOrigin";
        constexpr auto& ShowGrid = L"showGrid";
        constexpr auto& ShowLightMarkers = L"showLights";
        constexpr auto& ShowFrusta = L"showFrusta";
        constexpr auto& ShowShadows = L"showShadows";
        constexpr auto& ShowCascadeSplits = L"showCascadeSplits";
        constexpr auto& ShadowMapCascadeCount = L"shadowMapCascadeCount";
        constexpr auto& ShadowMapQuality = L"shadowMapQuality";
        constexpr auto& ShowDebuggingMenu = L"showDebuggingMenu";
        constexpr auto& MonitorFileSystem = L"monitorFileSystem";
    } // namespace Preferences

    namespace Visualization
    {
        constexpr auto PaddingRatio = 0.9;
        constexpr auto MaxPadding = 0.75;

        constexpr auto BlockHeight = 2.0f;
        constexpr auto RootBlockWidth = 1000.0f;
        constexpr auto RootBlockDepth = 1000.0f;
    } // namespace Visualization
} // namespace Constants

#endif // CONSTANTS
