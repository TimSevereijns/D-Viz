#include "nodePainterTests.h"

#include <Settings/nodePainter.h>
#include <Settings/settings.h>
#include <constants.h>

void NodePainterTests::initTestCase()
{
}

void NodePainterTests::cleanupTestCase()
{
}

void NodePainterTests::init()
{
    Settings::JsonDocument document;
    document.SetObject();

    auto& allocator = document.GetAllocator();

    rapidjson::Value array{ rapidjson::kArrayType };
    array.PushBack(128, allocator);
    array.PushBack(128, allocator);
    array.PushBack(128, allocator);

    rapidjson::Value object{ rapidjson::kObjectType };
    object.AddMember(".jpg", array.Move(), allocator);

    document.AddMember("Default", object.Move(), allocator);

    Settings::SaveToDisk(document, std::filesystem::current_path() / "colors.json");
}

void NodePainterTests::DetermineColorsFromSettingsOnDisk() const
{
    Settings::NodePainter painter;
    painter.SetActiveColorScheme("Default");

    const auto color = painter.DetermineColorFromExtension(".jpg");
    QVERIFY(color.has_value());

    constexpr auto expectedColor = Color::FromRGB(128, 128, 128);
    QCOMPARE(*color, expectedColor);
}

void NodePainterTests::GetBackEmptyOptionalOnEmptyMapping() const
{
    Settings::NodePainter painter;
    painter.SetActiveColorScheme("Default");

    const auto validMapping = painter.DetermineColorFromExtension(".foo");
    QCOMPARE(validMapping.has_value(), false);

    painter.SetActiveColorScheme("Nonexistent");
    const auto absentMapping = painter.DetermineColorFromExtension(".jpg");

    QCOMPARE(absentMapping.has_value(), false);
}

void NodePainterTests::ModifyActiveColorScheme() const
{
    constexpr auto& scheme = "Audio";

    Settings::NodePainter painter;
    painter.SetActiveColorScheme(scheme);

    const auto retrievedScheme = painter.GetActiveColorScheme();
    QCOMPARE(retrievedScheme, scheme);
}

void NodePainterTests::GenerateDefaultColorSchemeFile() const
{
    const auto path = std::filesystem::current_path() / "colors.json";
    if (std::filesystem::exists(path)) {
        std::filesystem::remove(path);
    }

    Settings::NodePainter painter;
    painter.SetActiveColorScheme("Images");
    const auto jpgMapping = painter.DetermineColorFromExtension(".jpg");
    QCOMPARE(jpgMapping.has_value(), true);
}

REGISTER_TEST(NodePainterTests)
