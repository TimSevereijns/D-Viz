#include <QApplication>

#include <bootstrapper.hpp>

#include "Utilities/multiTestHarness.h"

int main(int argc, char* argv[])
{
    QApplication app{ argc, argv };

    Bootstrapper::RegisterMetaTypes();
    Bootstrapper::InitializeLogs("-debug");

    const auto failedTests = MultiTest::RunAllTests(argc, argv);
    return failedTests;
}
