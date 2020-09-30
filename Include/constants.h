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
        [[maybe_unused]] constexpr auto Red = Detail::RGB(255, 0, 0);
        [[maybe_unused]] constexpr auto Green = Detail::RGB(0, 255, 0);
        [[maybe_unused]] constexpr auto Blue = Detail::RGB(0, 0, 255);
        [[maybe_unused]] constexpr auto BabyBlue = Detail::RGB(137, 207, 240);
        [[maybe_unused]] constexpr auto SteelBlue = Detail::RGB(70, 130, 180);
        [[maybe_unused]] constexpr auto CanaryYellow = Detail::RGB(255, 239, 0);
        [[maybe_unused]] constexpr auto HotPink = Detail::RGB(255, 105, 180);
        [[maybe_unused]] constexpr auto DeepPink = Detail::RGB(255, 20, 147);
        [[maybe_unused]] constexpr auto FileGreen = Detail::RGB(128, 255, 128);
        [[maybe_unused]] constexpr auto SlateGray = Detail::RGB(112, 128, 144);
        [[maybe_unused]] constexpr auto White = Detail::RGB(255, 255, 255);
        [[maybe_unused]] constexpr auto GrayBlue = Detail::RGB(64, 128, 191);
        [[maybe_unused]] constexpr auto PalePastelRed = Detail::RGB(255, 153, 148);
        [[maybe_unused]] constexpr auto PastelRed = Detail::RGB(255, 105, 97);

        inline namespace Alias
        {
            [[maybe_unused]] constexpr auto File = Colors::FileGreen;
            [[maybe_unused]] constexpr auto Directory = Colors::White;
            [[maybe_unused]] constexpr auto DeletedFile = Colors::PastelRed;
            [[maybe_unused]] constexpr auto DeletedDirectory = Colors::PalePastelRed;
            [[maybe_unused]] constexpr auto ModifiedFile = Colors::BabyBlue;
            [[maybe_unused]] constexpr auto ModifiedDirectory = Colors::SteelBlue;
            [[maybe_unused]] constexpr auto Selected = Colors::CanaryYellow;
            [[maybe_unused]] constexpr auto Highlighted = Colors::SlateGray;
            [[maybe_unused]] constexpr auto SchemeHighlight = Colors::GrayBlue;
        } // namespace Alias

    } // namespace Colors

    namespace ColorScheme
    {
        [[maybe_unused]] constexpr auto& Default = L"Default";
    }

    namespace Graphics
    {
        [[maybe_unused]] constexpr auto DesiredTimeBetweenFrames = 20;
    }

    namespace Input
    {
        [[maybe_unused]] constexpr auto MovementAmplification = 10.0;
        [[maybe_unused]] constexpr auto TriggerActuationThreshold = 0.2;
    } // namespace Input

    namespace Concurrency
    {
        [[maybe_unused]] constexpr auto ThreadLimit = 4u;
    }

    namespace Logging
    {
        [[maybe_unused]] constexpr auto& DefaultLog = "D-Viz";
        [[maybe_unused]] constexpr auto& FilesystemLog = "Filesystem";
    } // namespace Logging

    namespace Math
    {
        [[maybe_unused]] constexpr auto Pi = 3.14159265358979323846;
        [[maybe_unused]] constexpr auto RadiansToDegrees = 180.0 / Pi;
        [[maybe_unused]] constexpr auto DegreesToRadians = Pi / 180.0;
    } // namespace Math

    namespace Preferences
    {
        [[maybe_unused]] constexpr auto& ShowOrigin = L"showOrigin";
        [[maybe_unused]] constexpr auto& ShowGrid = L"showGrid";
        [[maybe_unused]] constexpr auto& ShowLightMarkers = L"showLights";
        [[maybe_unused]] constexpr auto& ShowFrusta = L"showFrusta";
        [[maybe_unused]] constexpr auto& ShowShadows = L"showShadows";
        [[maybe_unused]] constexpr auto& ShowCascadeSplits = L"showCascadeSplits";
        [[maybe_unused]] constexpr auto& ShadowMapCascadeCount = L"shadowMapCascadeCount";
        [[maybe_unused]] constexpr auto& ShadowMapQuality = L"shadowMapQuality";
        [[maybe_unused]] constexpr auto& ShowDebuggingMenu = L"showDebuggingMenu";
        [[maybe_unused]] constexpr auto& MonitorFileSystem = L"monitorFileSystem";
    } // namespace Preferences

    namespace Visualization
    {
        [[maybe_unused]] constexpr auto PaddingRatio = 0.9;
        [[maybe_unused]] constexpr auto MaxPadding = 0.75;

        [[maybe_unused]] constexpr auto BlockHeight = 2.0f;
        [[maybe_unused]] constexpr auto RootBlockWidth = 1000.0f;
        [[maybe_unused]] constexpr auto RootBlockDepth = 1000.0f;
    } // namespace Visualization

    const QVersionNumber Version{ 0, 3, 0 };
} // namespace Constants

#endif // CONSTANTS
