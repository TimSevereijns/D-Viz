#include <QApplication>

#include <bootstrapper.h>

#include "Utilities/multiTestHarness.h"

int main(int argc, char* argv[])
{
    QApplication app{ argc, argv };

    Bootstrapper::RegisterMetaTypes();
    Bootstrapper::InitializeLogs("-unit-testing");

    const auto failedTests = MultiTest::RunAllTests(argc, argv);
    return failedTests;
}
