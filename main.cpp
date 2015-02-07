#include "mainwindow.h"
#include <QApplication>

#include "diskScanner.h"
#include "tree.h"

#include <algorithm>
#include <iostream>
#include <memory>

#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QIODevice>
#include <QJsonObject>
#include <QJsonDocument>

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

   std::unique_ptr<Tree<std::string>> CreateSimpleStringBinaryTree()
   {
      std::unique_ptr<Tree<std::string>> tree(new Tree<std::string>("F"));
      tree->GetHead()->AppendChild("B")->AppendChild("A");
      tree->GetHead()->GetFirstChild()->AppendChild("D")->AppendChild("C");
      tree->GetHead()->GetFirstChild()->GetLastChild()->AppendChild("E");
      tree->GetHead()->AppendChild("G")->AppendChild("I")->AppendChild("H");

      return tree;
   }

   void QuickTreeTest()
   {
      auto tree = CreateSimpleStringBinaryTree();
      const std::vector<std::string> expectedTraversal { "H", "I", "G", "E", "C", "D", "A", "B",
                                                         "F" };

      int index = 0;
      bool traversalError = false;

      auto itr = tree->endPreOrder();
      for (--itr; itr != tree->beginPreOrder(); --itr)
      {
         if (itr->GetData() != expectedTraversal[index++])
         {
            traversalError = true;
            break;
         }
      }

      if (itr->GetData() != expectedTraversal[index])
      {
         traversalError = true;
      }

      std::cout << (traversalError ? "Error!" : "All O.K.") << std::endl;
   }

   void QuickDiskTest()
   {
      const std::wstring path {L"C:\\excluded"};
      auto scanner = DiskScanner(path);

      std::atomic<std::pair<std::uintmax_t, bool>> progress{std::make_pair(0, false)};
      scanner.ScanInNewThread(&progress);

      while (progress.load().second == false)
      {
         std::cout << "Files scanned so far: "
                   << progress.load().first << std::endl;
         std::this_thread::sleep_for(std::chrono::seconds(1));
      }

      scanner.JoinScanningThread();

      //scanner.PrintTree();
      scanner.PrintTreeMetadata();

      QJsonObject serializedTree;
      scanner.ToJSON(serializedTree);

      QJsonDocument jsonDocument{serializedTree};
      QByteArray serializerOutput = jsonDocument.toJson();

      QFile file("C:\\Users\\tsevereijns\\Desktop\\serializedTree.txt");
      file.open(QIODevice::WriteOnly);
      file.write(serializerOutput);
      file.close();
   }
}

int main(int argc, char* argv[])
{
   QApplication application(argc, argv);
   MainWindow mainWindow;
   mainWindow.show();

   QuickDiskTest();

   return application.exec();
}
