#include "mainwindow.h"
#include <QApplication>

template<typename T> class Tree;
struct VizNode;

int main(int argc, char* argv[])
{
   qRegisterMetaType<std::uintmax_t>("std::uintmax_t");
   qRegisterMetaType<std::shared_ptr<Tree<VizNode>>>("std::shared_ptr<Tree<VizNode>>");

   QApplication application{argc, argv};

   MainWindow mainWindow{/* parent =*/ 0};
   mainWindow.show();

   return application.exec();
}
