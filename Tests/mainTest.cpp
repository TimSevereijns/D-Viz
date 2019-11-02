#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include <QCoreApplication>
#include <QtTest>

#include <bootstrapper.hpp>

#include "fileSizeLiteralTests.hpp"

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    Bootstrapper::RegisterMetaTypes();
    Bootstrapper::InitializeLogs();

    QTEST_SET_MAIN_SOURCE_PATH
    int result = Catch::Session().run(argc, argv);

    return (result < 0xff ? result : 0xff);
}
