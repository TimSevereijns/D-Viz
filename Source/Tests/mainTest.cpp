#include <QApplication>

#include "multiTestHarness.h"

int main(int argc, char* argv[])
{
   QApplication app{ argc, argv };

   MultiTest::RunAllTests(argc, argv);

   return 0;
}
