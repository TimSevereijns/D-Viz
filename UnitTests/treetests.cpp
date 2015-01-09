#include <QtTest/QtTest>

#include <../tree.h>

class TreeTests: public QObject
{
   Q_OBJECT

   private slots:
      void IntegerTreeCreation();
      void PostOrderIteratorTestOne();
};

void TreeTests::IntegerTreeCreation()
{
   std::unique_ptr<Tree<int>> tree(new Tree<int>(99));
   QVERIFY(tree.get());
}

void TreeTests::PostOrderIteratorTestOne()
{
   std::unique_ptr<Tree<int>> tree(new Tree<int>(99));
   tree->GetHead()->AppendChild(1);
   tree->GetHead()->AppendChild(2);

   std::shared_ptr<TreeNode<int>> head = tree->GetHead();
   int data = head->GetData();
   QVERIFY(data == 99);
}

QTEST_MAIN(TreeTests)
#include "treetests.moc"
