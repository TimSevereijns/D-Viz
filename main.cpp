#include "mainwindow.h"
#include <QApplication>

#include "tree.h"

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

   Tree<int>::PostOrderIterator postOrder = tree->begin();

   postOrder++;
   if (postOrder != tree->end())
   {
      std::cout << *postOrder << std::endl; // Should be 33
   }

   postOrder++;
   if (postOrder != tree->end())
   {
      std::cout << *postOrder << std::endl; // Should be 3
   }

   postOrder++;
   if (postOrder != tree->end())
   {
      std::cout << *postOrder << std::endl; // Should be 0
   }

   postOrder++;
   if (postOrder != tree->end())
   {
      std::cout << *postOrder << std::endl; // Should be 1
   }

   return application.exec();
}
