#include "mainwindow.h"
#include <QApplication>

#include "diskScanner.h"
#include "tree.h"

#include <algorithm>
#include <codecvt>
#include <iostream>
#include <locale>
#include <memory>
#include <string>

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

   std::unique_ptr<Tree<int>> CreateSimpleIntegerBinaryTree()
   {
      std::unique_ptr<Tree<int>> tree = std::make_unique<Tree<int>>(99);
      tree->GetHead()->AppendChild(3);
      tree->GetHead()->AppendChild(2);
      tree->GetHead()->AppendChild(7);
      tree->GetHead()->AppendChild(6);
      tree->GetHead()->AppendChild(4);
      tree->GetHead()->AppendChild(5);
      tree->GetHead()->AppendChild(9);
      tree->GetHead()->AppendChild(8);
      tree->GetHead()->AppendChild(1);

      return tree;
   }

   void QuickDiskTest()
   {
      const std::wstring path {L"C:\\Users\\tsevereijns\\Desktop"};
      auto scanner = DiskScanner(path);

      std::atomic<std::pair<std::uintmax_t, bool>> progress{std::make_pair(0, false)};
      scanner.ScanInNewThread(&progress);

      while (progress.load().second == false)
      {
         std::cout.imbue(std::locale(""));
         std::cout << "Files scanned so far: " << progress.load().first << std::endl;
         std::this_thread::sleep_for(std::chrono::seconds(1));
      }

      scanner.JoinScanningThread();
      scanner.PrintTreeMetadata();
      scanner.PrintTree();

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
   MainWindow mainWindow{/* parent =*/ 0};
   mainWindow.show();

   //QuickSortingTest();

   return application.exec();
}
