#include <QApplication>

#include <bootstrapper.hpp>

#include "multiTestHarness.h"

int main(int argc, char* argv[])
{
    QApplication app{ argc, argv };

    Bootstrapper::RegisterMetaTypes();
    Bootstrapper::InitializeLogs("-debug");

    const auto failures = MultiTest::RunAllTests(argc, argv);
    return failures;
}
