#include <QtTest/QtTest>

#include <../tree.h>

namespace {
   std::unique_ptr<Tree<int>> CreateSimpleIntegerBinaryTree()
   {
      std::unique_ptr<Tree<int>> tree(new Tree<int>(99));
      tree->GetHead()->AppendChild(1);
      tree->GetHead()->AppendChild(2);

      tree->GetHead()->GetFirstChild()->AppendChild(11);
      tree->GetHead()->GetFirstChild()->AppendChild(12);

      return tree;
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
}

class TreeTests: public QObject
{
   Q_OBJECT

   private slots:
      void IntegerTreeCreation();
      void StringTreeCreation();

      void TreeSize();

      void PostOrderIteratorTestOne();
};

void TreeTests::IntegerTreeCreation()
{
   std::unique_ptr<Tree<int>> tree(new Tree<int>(99));

   const int data = tree->GetHead()->GetData();
   QVERIFY(data == 99);
}

void TreeTests::StringTreeCreation()
{
   std::unique_ptr<Tree<std::string>> tree(new Tree<std::string>("root"));

   const std::string& data = tree->GetHead()->GetData();
   QVERIFY(data == "root");
}

void TreeTests::TreeSize()
{
   std::unique_ptr<Tree<int>> tree = CreateSimpleIntegerBinaryTree();

   QVERIFY(tree->Size() == 5);
}

void TreeTests::PostOrderIteratorTestOne()
{
   std::unique_ptr<Tree<std::string>> tree = CreateSimpleStringBinaryTree();

   std::vector<std::string> iterationOutput;
   std::copy(std::begin(*tree), std::end(*tree), std::back_inserter(iterationOutput));

   const std::vector<std::string> expectedTraversal { "A", "C", "E", "D", "B", "H", "I", "G", "F" };

   QVERIFY(tree->Size() == iterationOutput.size());
}

QTEST_MAIN(TreeTests)
#include "treetests.moc"
