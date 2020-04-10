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
    enum struct SizePrefix
    {
        Binary,
        Decimal
    };

    namespace Colors
    {
        constexpr static auto Red = Detail::RGB(255, 0, 0);
        constexpr static auto Green = Detail::RGB(0, 255, 0);
        constexpr static auto Blue = Detail::RGB(0, 0, 255);
        constexpr static auto BabyBlue = Detail::RGB(137, 207, 240);
        constexpr static auto CanaryYellow = Detail::RGB(255, 239, 0);
        constexpr static auto HotPink = Detail::RGB(255, 105, 180);
        constexpr static auto FileGreen = Detail::RGB(128, 255, 128);
        constexpr static auto SlateGray = Detail::RGB(112, 128, 144);
        constexpr static auto White = Detail::RGB(255, 255, 255);
        constexpr static auto Coral = Detail::RGB(255, 127, 80);
    } // namespace Colors

    namespace ColorScheme
    {
        constexpr static auto& Default = L"Default";
    }

    namespace Graphics
    {
        constexpr static auto DesiredTimeBetweenFrames = 20;
    }

    namespace Input
    {
        constexpr static auto MovementAmplification = 10.0;
        constexpr static auto TriggerActuationThreshold = 0.2;
    } // namespace Input

    namespace Concurrency
    {
        constexpr static auto ThreadLimit = 4u;
    }

    namespace Logging
    {
        constexpr static auto& DefaultLog = "D-Viz";
        constexpr static auto& FilesystemLog = "Filesystem";
    } // namespace Logging

    namespace Math
    {
        constexpr static auto Pi = 3.14159265358979323846;
        constexpr static auto RadiansToDegrees = 180.0 / Pi;
        constexpr static auto DegreesToRadians = Pi / 180.0;
    } // namespace Math

    namespace Preferences
    {
        constexpr static auto& ShowOrigin = L"showOrigin";
        constexpr static auto& ShowGrid = L"showGrid";
        constexpr static auto& ShowLightMarkers = L"showLights";
        constexpr static auto& ShowFrusta = L"showFrusta";
        constexpr static auto& ShowShadows = L"showShadows";
        constexpr static auto& ShowCascadeSplits = L"showCascadeSplits";
        constexpr static auto& ShadowMapCascadeCount = L"shadowMapCascadeCount";
        constexpr static auto& ShadowMapQuality = L"shadowMapQuality";
        constexpr static auto& ShowDebuggingMenu = L"showDebuggingMenu";
        constexpr static auto& MonitorFileSystem = L"monitorFileSystem";
    } // namespace Preferences

    namespace Visualization
    {
        constexpr static auto PaddingRatio = 0.9;
        constexpr static auto MaxPadding = 0.75;

        constexpr static auto BlockHeight = 2.0f;
        constexpr static auto RootBlockWidth = 1000.0f;
        constexpr static auto RootBlockDepth = 1000.0f;
    } // namespace Visualization
} // namespace Constants

#endif // CONSTANTS
