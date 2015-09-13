#include "mainwindow.h"
#include <QApplication>

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
   qRegisterMetaType<std::uintmax_t>("std::uintmax_t");

   QApplication application(argc, argv);
   MainWindow mainWindow{/* parent =*/ 0};
   mainWindow.show();

   return application.exec();
}
