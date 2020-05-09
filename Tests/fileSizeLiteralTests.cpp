#include "fileSizeLiteralTests.h"

#include <literals.h>

void FileSizeLiteralTests::initTestCase()
{
}

void FileSizeLiteralTests::cleanupTestCase()
{
}

void FileSizeLiteralTests::init()
{
}

void FileSizeLiteralTests::KilobytesToBytes() const
{
    using namespace Literals::Numeric::Decimal;

    QCOMPARE(1_KB, 1'000ull);
    QCOMPARE(10_KB, 10'000ull);
    QCOMPARE(100_KB, 100'000ull);
}

void FileSizeLiteralTests::MegabytesToBytes() const
{
    using namespace Literals::Numeric::Decimal;

    QCOMPARE(1_MB, 1'000'000ull);
    QCOMPARE(10_MB, 10'000'000ull);
    QCOMPARE(100_MB, 100'000'000ull);
}

void FileSizeLiteralTests::GigabytesToBytes() const
{
    using namespace Literals::Numeric::Decimal;

    QCOMPARE(1_GB, 1'000'000'000ull);
    QCOMPARE(10_GB, 10'000'000'000ull);
    QCOMPARE(100_GB, 100'000'000'000ull);
}

void FileSizeLiteralTests::TerabytesToBytes() const
{
    using namespace Literals::Numeric::Decimal;

    QCOMPARE(1_TB, 1'000'000'000'000ull);
    QCOMPARE(10_TB, 10'000'000'000'000ull);
    QCOMPARE(100_TB, 100'000'000'000'000ull);
}

void FileSizeLiteralTests::KibibytesToBytes() const
{
    using namespace Literals::Numeric::Binary;

    QCOMPARE(1_KiB, 1'024ull);
    QCOMPARE(10_KiB, 10'240ull);
    QCOMPARE(100_KiB, 102'400ull);
}

void FileSizeLiteralTests::MebibytesToBytes() const
{
    using namespace Literals::Numeric::Binary;

    QCOMPARE(1_MiB, 1048576ull);
    QCOMPARE(10_MiB, 10'485'760ull);
    QCOMPARE(100_MiB, 104'857'600ull);
}

void FileSizeLiteralTests::GibibytesToBytes() const
{
    using namespace Literals::Numeric::Binary;

    QCOMPARE(1_GiB, 1'073'741'824ull);
    QCOMPARE(10_GiB, 10'737'418'240ull);
    QCOMPARE(100_GiB, 107'374'182'400ull);
}

void FileSizeLiteralTests::TebibytesToBytes() const
{
    using namespace Literals::Numeric::Binary;

    QCOMPARE(1_TiB, 1'099'511'627'776ull);
    QCOMPARE(10_TiB, 10'995'116'277'760ull);
    QCOMPARE(100_TiB, 109'951'162'777'600ull);
}

REGISTER_TEST(FileSizeLiteralTests)
