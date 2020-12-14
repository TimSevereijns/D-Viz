#include <QApplication>

#include <bootstrapper.h>

#include "Utilities/multiTestHarness.h"

int main(int argc, char* argv[])
{
    [[maybe_unused]] const auto locale = std::locale::global(std::locale("en_US.UTF-8"));

    QApplication app{ argc, argv };

    Bootstrapper::RegisterMetaTypes();
    Bootstrapper::InitializeLogs("-unit-testing");

    const auto failedTests = MultiTest::RunAllTests(argc, argv);
    return failedTests;
}
