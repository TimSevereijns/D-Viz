#include "mainwindow.h"
#include <QApplication>

#include "tree.h"

#include <algorithm>
#include <iostream>
#include <memory>

namespace {
   template<class T>
   unsigned int Size(const TreeNode<T>& node)
   {
      unsigned int count = 0;
      for (auto itr = beginLeaf(); itr != endLeaf(); ++itr)
      {
         count++;
      }

      return count;
   }
}

int main(int argc, char* argv[])
{
   QApplication application(argc, argv);
   MainWindow mainWindow;
   mainWindow.show();

   std::unique_ptr<Tree<std::string>> tree(new Tree<std::string>("F"));
   tree->GetHead()->AppendChild("B")->AppendChild("A");
   tree->GetHead()->GetFirstChild()->AppendChild("D")->AppendChild("C");
   tree->GetHead()->GetFirstChild()->GetLastChild()->AppendChild("E");
   tree->GetHead()->AppendChild("G")->AppendChild("I")->AppendChild("H");

   const std::vector<std::string> expectedTraversal { "H", "E", "C", "A" };
   int index = 0;

   bool traversalError = false;
   Tree<std::string>::LeafIterator itr = tree->endLeaf();

   for (--itr; itr != tree->beginLeaf(); --itr)
   {
      if (itr->GetData() != expectedTraversal[index++])
      {
         traversalError = true;
         break;
      }
   }

   // Don't forget the last one:
   if (itr->GetData() != expectedTraversal[index])
   {
      traversalError = true;
   }

   return application.exec();
}
