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

//   std::cout << tree->GetHead()->GetData() << std::endl;
//   std::cout << tree->GetHead()->GetFirstChild()->GetData() << std::endl;
//   std::cout << tree->GetHead()->GetFirstChild()->GetNextSibling()->GetData() << std::endl;
//   std::cout << tree->GetHead()->GetFirstChild()->GetNextSibling()->GetNextSibling()->GetData() << std::endl;
//   std::cout << tree->GetHead()->GetFirstChild()->GetParent()->GetData() << std::endl;
//   std::cout << tree->GetHead()->GetLastChild()->GetParent()->GetData() << std::endl;
//   std::cout << tree->GetHead()->GetFirstChild()->GetFirstChild()->GetData() << std::endl;

   Tree<int>::PostOrderIterator itr = tree->end();

   std::cout << *itr << std::endl;

//   std::for_each(std::begin(*tree), std::end(*tree),
//      [] (int data)
//   {
//      std::cout << data << std::endl;
//   });

   return application.exec();
}
