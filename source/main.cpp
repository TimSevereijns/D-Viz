#include "Windows/mainWindow.h"

#include <QApplication>

template<typename NodeDataType>
class Tree;

struct VizNode;

int main(int argc, char* argv[])
{
   qRegisterMetaType<std::uintmax_t>("std::uintmax_t");
   qRegisterMetaType<std::shared_ptr<Tree<VizNode>>>("std::shared_ptr<Tree<VizNode>>");

   QApplication application{ argc, argv };

   Controller model{ };
   MainWindow view{ model, nullptr };
   model.SetView(&view);
   view.show();

   return application.exec();
}
