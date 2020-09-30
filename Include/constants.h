#ifndef CONSTANTS
#define CONSTANTS

#include <QVector3D>
#include <QVersionNumber>

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
        [[maybe_unused]] inline constexpr auto Red = Detail::RGB(255, 0, 0);
        [[maybe_unused]] inline constexpr auto Green = Detail::RGB(0, 255, 0);
        [[maybe_unused]] inline constexpr auto Blue = Detail::RGB(0, 0, 255);
        [[maybe_unused]] inline constexpr auto BabyBlue = Detail::RGB(137, 207, 240);
        [[maybe_unused]] inline constexpr auto SteelBlue = Detail::RGB(70, 130, 180);
        [[maybe_unused]] inline constexpr auto CanaryYellow = Detail::RGB(255, 239, 0);
        [[maybe_unused]] inline constexpr auto HotPink = Detail::RGB(255, 105, 180);
        [[maybe_unused]] inline constexpr auto DeepPink = Detail::RGB(255, 20, 147);
        [[maybe_unused]] inline constexpr auto FileGreen = Detail::RGB(128, 255, 128);
        [[maybe_unused]] inline constexpr auto SlateGray = Detail::RGB(112, 128, 144);
        [[maybe_unused]] inline constexpr auto White = Detail::RGB(255, 255, 255);
        [[maybe_unused]] inline constexpr auto GrayBlue = Detail::RGB(64, 128, 191);
        [[maybe_unused]] inline constexpr auto PalePastelRed = Detail::RGB(255, 153, 148);
        [[maybe_unused]] inline constexpr auto PastelRed = Detail::RGB(255, 105, 97);

        inline namespace Alias
        {
            [[maybe_unused]] inline constexpr auto File = Colors::FileGreen;
            [[maybe_unused]] inline constexpr auto Directory = Colors::White;
            [[maybe_unused]] inline constexpr auto DeletedFile = Colors::PastelRed;
            [[maybe_unused]] inline constexpr auto DeletedDirectory = Colors::PalePastelRed;
            [[maybe_unused]] inline constexpr auto ModifiedFile = Colors::BabyBlue;
            [[maybe_unused]] inline constexpr auto ModifiedDirectory = Colors::SteelBlue;
            [[maybe_unused]] inline constexpr auto Selected = Colors::CanaryYellow;
            [[maybe_unused]] inline constexpr auto Highlighted = Colors::SlateGray;
            [[maybe_unused]] inline constexpr auto SchemeHighlight = Colors::GrayBlue;
        } // namespace Alias

    } // namespace Colors

    namespace ColorScheme
    {
        [[maybe_unused]] inline constexpr auto& Default = L"Default";
    }

    namespace Graphics
    {
        [[maybe_unused]] inline constexpr auto DesiredTimeBetweenFrames = 20;
    }

    namespace Input
    {
        [[maybe_unused]] inline constexpr auto MovementAmplification = 10.0;
        [[maybe_unused]] inline constexpr auto TriggerActuationThreshold = 0.2;
    } // namespace Input

    namespace Concurrency
    {
        [[maybe_unused]] inline constexpr auto ThreadLimit = 4u;
    }

    namespace Logging
    {
        [[maybe_unused]] inline constexpr auto& DefaultLog = "D-Viz";
        [[maybe_unused]] inline constexpr auto& FilesystemLog = "Filesystem";
    } // namespace Logging

    namespace Math
    {
        [[maybe_unused]] inline constexpr auto Pi = 3.14159265358979323846;
        [[maybe_unused]] inline constexpr auto RadiansToDegrees = 180.0 / Pi;
        [[maybe_unused]] inline constexpr auto DegreesToRadians = Pi / 180.0;
    } // namespace Math

    namespace Preferences
    {
        [[maybe_unused]] inline constexpr auto& ShowOrigin = L"showOrigin";
        [[maybe_unused]] inline constexpr auto& ShowGrid = L"showGrid";
        [[maybe_unused]] inline constexpr auto& ShowLightMarkers = L"showLights";
        [[maybe_unused]] inline constexpr auto& ShowFrusta = L"showFrusta";
        [[maybe_unused]] inline constexpr auto& ShowShadows = L"showShadows";
        [[maybe_unused]] inline constexpr auto& ShowCascadeSplits = L"showCascadeSplits";
        [[maybe_unused]] inline constexpr auto& ShadowMapCascadeCount = L"shadowMapCascadeCount";
        [[maybe_unused]] inline constexpr auto& ShadowMapQuality = L"shadowMapQuality";
        [[maybe_unused]] inline constexpr auto& ShowDebuggingMenu = L"showDebuggingMenu";
        [[maybe_unused]] inline constexpr auto& MonitorFileSystem = L"monitorFileSystem";
    } // namespace Preferences

    namespace Visualization
    {
        [[maybe_unused]] inline constexpr auto PaddingRatio = 0.9;
        [[maybe_unused]] inline constexpr auto MaxPadding = 0.75;

        [[maybe_unused]] inline constexpr auto BlockHeight = 2.0f;
        [[maybe_unused]] inline constexpr auto RootBlockWidth = 1000.0f;
        [[maybe_unused]] inline constexpr auto RootBlockDepth = 1000.0f;
    } // namespace Visualization

    const QVersionNumber Version{ 0, 3, 0 };
} // namespace Constants

#endif // CONSTANTS
