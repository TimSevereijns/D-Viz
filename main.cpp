#include "mainwindow.h"
#include <QApplication>

#include "tree.h"

#include <algorithm>
#include <iostream>
#include <memory>

int main(int argc, char* argv[])
{
   QApplication application(argc, argv);
   MainWindow mainWindow;
   mainWindow.show();

   std::unique_ptr<Tree<int>> tree(new Tree<int>(99));
   tree->GetHead()->AppendChild(1);
   tree->GetHead()->AppendChild(2);
   tree->GetHead()->PrependChild(0);
   tree->GetHead()->GetFirstChild()->AppendChild(3)->AppendChild(33);

   Tree<int>::PostOrderIterator itr = tree->end();

   --itr;

   std::cout << *itr << std::endl;

   return application.exec();
}
