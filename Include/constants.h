#ifndef CONSTANTS
#define CONSTANTS

#include <QVector3D>
#include <QVersionNumber>

#include "Utilities/color.h"
#include "literals.h"

#ifdef Q_OS_WIN
#undef RGB
#endif // Q_OS_WIN

namespace Constants
{
    enum struct SizePrefix
    {
        Binary,
        Decimal
    };

    namespace Colors
    {
        [[maybe_unused]] inline constexpr auto Red = Color::FromRGB(255, 0, 0);
        [[maybe_unused]] inline constexpr auto Green = Color::FromRGB(0, 255, 0);
        [[maybe_unused]] inline constexpr auto Blue = Color::FromRGB(0, 0, 255);
        [[maybe_unused]] inline constexpr auto BabyBlue = Color::FromRGB(137, 207, 240);
        [[maybe_unused]] inline constexpr auto SteelBlue = Color::FromRGB(70, 130, 180);
        [[maybe_unused]] inline constexpr auto CanaryYellow = Color::FromRGB(255, 239, 0);
        [[maybe_unused]] inline constexpr auto HotPink = Color::FromRGB(255, 105, 180);
        [[maybe_unused]] inline constexpr auto DeepPink = Color::FromRGB(255, 20, 147);
        [[maybe_unused]] inline constexpr auto FileGreen = Color::FromRGB(128, 255, 128);
        [[maybe_unused]] inline constexpr auto SlateGray = Color::FromRGB(112, 128, 144);
        [[maybe_unused]] inline constexpr auto White = Color::FromRGB(255, 255, 255);
        [[maybe_unused]] inline constexpr auto GrayBlue = Color::FromRGB(64, 128, 191);
        [[maybe_unused]] inline constexpr auto PalePastelRed = Color::FromRGB(255, 153, 148);
        [[maybe_unused]] inline constexpr auto PastelRed = Color::FromRGB(255, 105, 97);

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
        [[maybe_unused]] inline constexpr auto& Default = "Default";
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
        [[maybe_unused]] inline constexpr auto& ShowOrigin = "showOrigin";
        [[maybe_unused]] inline constexpr auto& ShowGrid = "showGrid";
        [[maybe_unused]] inline constexpr auto& ShowLightMarkers = "showLights";
        [[maybe_unused]] inline constexpr auto& ShowFrusta = "showFrusta";
        [[maybe_unused]] inline constexpr auto& ShowShadows = "showShadows";
        [[maybe_unused]] inline constexpr auto& ShowCascadeSplits = "showCascadeSplits";
        [[maybe_unused]] inline constexpr auto& ShadowMapCascadeCount = "shadowMapCascadeCount";
        [[maybe_unused]] inline constexpr auto& ShadowMapQuality = "shadowMapQuality";
        [[maybe_unused]] inline constexpr auto& ShowDebuggingMenu = "showDebuggingMenu";
        [[maybe_unused]] inline constexpr auto& MonitorFileSystem = "monitorFileSystem";
        [[maybe_unused]] inline constexpr auto& UseDarkMode = "useDarkMode";
    } // namespace Preferences

    namespace Treemap
    {
        [[maybe_unused]] inline constexpr auto PaddingRatio = 0.9;
        [[maybe_unused]] inline constexpr auto MaxPadding = 0.75;

        [[maybe_unused]] inline constexpr auto BlockHeight = 2.0;
        [[maybe_unused]] inline constexpr auto RootBlockWidth = 1000.0;
        [[maybe_unused]] inline constexpr auto RootBlockDepth = 1000.0;
    } // namespace Treemap

    namespace Units
    {
        [[maybe_unused]] inline constexpr auto& Bytes = "bytes";
    }

    const QVersionNumber Version{ 0, 5, 0 };
} // namespace Constants

#endif // CONSTANTS
