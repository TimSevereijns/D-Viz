#include "mainwindow.h"
#include <QApplication>

#include "diskScanner.h"
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

//      const auto symPath = boost::filesystem::path(L"C:\\excluded\\source\\build");
//      const bool answer = (boost::filesystem::is_symlink(symPath));
//      std::cout << (answer ? "true" : "false") << std::endl;

      auto scanner = DiskScanner(path);

      std::atomic<std::pair<std::uintmax_t, bool>> progress{std::make_pair(0, false)};
      scanner.ScanInNewThread(&progress);
      scanner.JoinScanningThread();

      scanner.PrintTree();
      scanner.PrintTreeMetadata();
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
