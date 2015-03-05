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

   void QuickTreeSortingTest()
   {
      auto tree = CreateSimpleIntegerBinaryTree();
      tree->GetHead()->SortChildren(
         [] (TreeNode<int>& lhs, TreeNode<int>& rhs)
      {
         return lhs.GetData() < rhs.GetData();
      });

      const std::vector<int> expectedTraversal{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };

      bool traversalError = false;
      auto child = tree->GetHead()->GetFirstChild();

      std::for_each(std::begin(expectedTraversal), std::end(expectedTraversal),
         [&] (const int expected)
      {
         if (child->GetData() != expected)
         {
            traversalError = true;
         }

         child = child->GetNextSibling();
      });

      std::cout << (traversalError ? "Error!" : "All O.K.") << std::endl;
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

   void QuickSortingTest()
   {
      //std::unique_ptr<Tree<int>> tree = CreateIntegerTreeOfUnsortedNodes();

      std::unique_ptr<Tree<int>> tree(new Tree<int>(999));
      tree->GetHead()->AppendChild(634);
//      tree->GetHead()->GetFirstChild()->AppendChild(34);
//      tree->GetHead()->GetFirstChild()->AppendChild(13);
//      tree->GetHead()->GetFirstChild()->AppendChild(89);
//      tree->GetHead()->GetFirstChild()->AppendChild(3);
//      tree->GetHead()->GetFirstChild()->AppendChild(1);
//      tree->GetHead()->GetFirstChild()->AppendChild(0);
//      tree->GetHead()->GetFirstChild()->AppendChild(-5);

      tree->GetHead()->AppendChild(375);
      tree->GetHead()->AppendChild(173);
      tree->GetHead()->AppendChild(128);

      bool sortingError = false;
      int lastItem = -999;

      //PrintTree(*tree.get());

      // Sort:
      std::for_each(std::begin(*tree), std::end(*tree),
         [] (TreeNode<int>& node)
      {
         node.SortChildren([] (const TreeNode<int>& lhs, const TreeNode<int>& rhs)
            { return lhs.GetData() < rhs.GetData(); });
      });

      //PrintTree(*tree.get());

      // Verify:
      std::for_each(std::begin(*tree), std::end(*tree),
         [&] (TreeNode<int>& node)
      {
         if (!node.HasChildren())
         {
            return;
         }

         auto child = node.GetFirstChild();
         while (child)
         {
            if (child->GetData() < lastItem)
            {
               sortingError = true;
               lastItem = child->GetData();
            }

            child = child->GetNextSibling();
         }
      });
   }
}

int main(int argc, char* argv[])
{
   QApplication application(argc, argv);
   //MainWindow mainWindow;
   //mainWindow.show();

   //QuickDiskTest();
   //QuickTreeSortingTest();

   QuickSortingTest();

   return application.exec();
}
