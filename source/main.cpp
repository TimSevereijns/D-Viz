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

   Controller controller{ };
   MainWindow window{ controller, nullptr };
   controller.SetView(&window);
   window.show();

   return application.exec();
}
