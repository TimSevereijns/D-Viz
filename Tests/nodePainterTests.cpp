#include "nodePainterTests.h"

#include <Settings/nodePainter.h>
#include <Settings/settings.h>

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

    rapidjson::GenericValue<rapidjson::UTF16<>> array{ rapidjson::kArrayType };
    array.PushBack(128, allocator);
    array.PushBack(128, allocator);
    array.PushBack(128, allocator);

    rapidjson::GenericValue<rapidjson::UTF16<>> object{ rapidjson::kObjectType };
    object.AddMember(L".jpg", array.Move(), allocator);

    document.AddMember(L"Default", object.Move(), allocator);

    Settings::SaveToDisk(document, std::filesystem::current_path() / L"colors.json");
}

void NodePainterTests::ReadColorsFromDisk() const
{
    Settings::NodePainter painter;
    painter.SetColorScheme(L"Default");
    const auto color = painter.DetermineColorFromExtension(L".jpg");

    QVERIFY(color.has_value());
}

void NodePainterTests::ModifyActiveColorScheme() const
{
    constexpr auto& scheme = L"Audio";

    Settings::NodePainter painter;
    painter.SetColorScheme(scheme);
    const auto retrievedScheme = painter.GetActiveColorScheme();

    QCOMPARE(retrievedScheme, scheme);
}

REGISTER_TEST(NodePainterTests)
