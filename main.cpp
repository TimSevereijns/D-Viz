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

   std::unique_ptr<Tree<std::string>> tree(new Tree<std::string>("F"));
   tree->GetHead()->AppendChild("B")->AppendChild("A");
   tree->GetHead()->GetFirstChild()->AppendChild("D")->AppendChild("C");
   tree->GetHead()->GetFirstChild()->GetLastChild()->AppendChild("E");

   tree->GetHead()->AppendChild("G")->AppendChild("I")->AppendChild("H");

   const std::vector<std::string> expectedTraversal { "F", "G", "I", "H", "B", "D", "E", "C", "A" };

   int index = 0;
   bool traversalError = false;

   for (auto itr = std::rbegin(*tree); itr != std::rend(*tree); ++itr)
   {
      if (*itr != expectedTraversal[index++])
      {
         traversalError = true;
         break;
      }
   }

   return application.exec();
}
