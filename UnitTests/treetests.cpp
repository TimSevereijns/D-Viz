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

   std::unique_ptr<Tree<std::string>> CreateRootNodeWithManyChildren()
   {
      std::unique_ptr<Tree<std::string>> tree(new Tree<std::string>("root"));
      tree->GetHead()->AppendChild("A");
      tree->GetHead()->AppendChild("B");
      tree->GetHead()->AppendChild("C");
      tree->GetHead()->AppendChild("D");
      tree->GetHead()->AppendChild("E");

      return tree;
   }
}

/**
 * @brief The TreeTests class provides unit tests for the n-ary Tree class, and its supporting
 * TreeNode class.
 */
class TreeTests: public QObject
{
   Q_OBJECT

   private slots:
      /**
       * @brief IntegerTreeCreation creates a tree with an integer value stored in the head, and
       * verifies that this value can be correctly retrieved from the head node.
       */
      void IntegerTreeCreation();

      /**
       * @brief StringTreeCreation creates a tree with standard narrow string stored in the head,
       * and verifies that this value can be correctly retrieved from the head node.
       */
      void StringTreeCreation();

      /**
       * @brief TreeSize creates a simple binary tree, and then verifies that the size of the tree
       * is correctly computed.
       */
      void TreeSize();

      /**
       * @brief SubTreeSize
       */
      void SubTreeSize();

      /**
       * @brief GetFirstChild creates a new tree with half a dozen children, and verifies that
       * the node is able retrieve its first child correctly.
       */
      void GetFirstChild();

      /**
       * @brief GetLastChild creates a new tree with half a dozen children, and verifies that the
       * node is able to retrieve its last child correctly.
       */
      void GetLastChild();

      /**
       * @brief CountLeafNodes creates a simple binary tree and verifies that the head node is able
       * to correctly count all of its leaf nodes.
       */
      void CountLeafNodes();

      /**
       * @brief PostOrderIteratorOfSimpleBinaryTree creates a simple binary tree, and the verifies
       * that the Post Order Iterator is able to traverse the tree correctly.
       */
      void PostOrderTraversalOfSimpleBinaryTree();

      /**
       * @brief PostOrderIteratorTraversalOfLeftDegenerateBinaryTree creates a linked-list by
       * continually prepending a node to the previously prepended node, and then verifies that the
       * Post Order Iterator is able to traverse the resulting tree correctly.
       */
      void PostOrderTraversalOfLeftDegenerateBinaryTree();

      /**
       * @brief PostOrderIteratorTraversalOfRightDegenerateBinaryTree creates a linked-list by
       * continually appending a node to the previously appended node, and then verifies that the
       * Post Order Iterator is able to traverse the resulting tree correctly.
       */
      void PostOrderTraversalOfRightDegenerateBinaryTree();

      /**
       * @brief PostOrderTraversalFromEndToBegin creates a simple binary tree, and then uses the
       * pre-decrement operator to traverse the tree from the end to the beginning of the tree.
       */
      void PostOrderTraversalFromEndToBegin();

      /**
       * @brief ReversePostOrderTraversalOfSimpleBinaryTree creates a simple binary tree, and the
       * verifies that the reverse Post Order Iterator is able to traverse the tree correctly.
       */
      void ReversePostOrderTraversalOfSimpleBinaryTree();

      /**
       * @brief PostOrderTraversalForwardsAndBackwards creates a simple binary tree, and then
       * increments and decrements the Post Order Iterator randomly to verify that mixed traversal
       * works correctly.
       */
      void PostOrderTraversalForwardsAndBackwards();

      /**
       * @brief SiblingTraversal creates a tree containing a root node with five child nodes, and
       * then traverses over those five sibling nodes, starting with the first child.
       */
      void SiblingTraversal();

      /**
       * @brief LeafTraversalOfSimpleBinaryTree creates a simple binary tree, and the traverses over
       * the leaf nodes in that tree.
       */
      void LeafTraversalOfSimpleBinaryTree();
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
   std::unique_ptr<Tree<std::string>> tree = CreateSimpleStringBinaryTree();

   QVERIFY(tree->Size() == 9);
}

void TreeTests::SubTreeSize()
{
   std::unique_ptr<Tree<std::string>> tree = CreateSimpleStringBinaryTree();
   auto itr = std::begin(*tree);
   itr++; itr++; itr++; itr++;

   //Tree<std::string>::Size(*itr);
}

void TreeTests::GetFirstChild()
{
   std::unique_ptr<Tree<std::string>> tree(new Tree<std::string>("root"));
   tree->GetHead()->AppendChild("A");
   tree->GetHead()->AppendChild("B");
   tree->GetHead()->AppendChild("C");
   tree->GetHead()->AppendChild("D");
   tree->GetHead()->AppendChild("E");
   tree->GetHead()->AppendChild("F");

   QVERIFY(tree->GetHead()->GetFirstChild()->GetData() == "A");
}

void TreeTests::GetLastChild()
{
   std::unique_ptr<Tree<std::string>> tree(new Tree<std::string>("root"));
   tree->GetHead()->AppendChild("A");
   tree->GetHead()->AppendChild("B");
   tree->GetHead()->AppendChild("C");
   tree->GetHead()->AppendChild("D");
   tree->GetHead()->AppendChild("E");
   tree->GetHead()->AppendChild("F");

   QVERIFY(tree->GetHead()->GetLastChild()->GetData() == "F");
}

void TreeTests::CountLeafNodes()
{
   std::unique_ptr<Tree<std::string>> tree = CreateSimpleStringBinaryTree();

   QVERIFY(tree->CountLeafNodes() == 4);
}

void TreeTests::PostOrderTraversalOfSimpleBinaryTree()
{
   std::unique_ptr<Tree<std::string>> tree = CreateSimpleStringBinaryTree();

   const std::vector<std::string> expectedTraversal { "A", "C", "E", "D", "B", "H", "I", "G", "F" };
   std::vector<std::string> setDifference;

   std::set_difference(std::begin(*tree), std::end(*tree), std::begin(expectedTraversal),
                       std::end(expectedTraversal), std::back_inserter(setDifference));

   QVERIFY(setDifference.size() == 0);
}

void TreeTests::PostOrderTraversalOfLeftDegenerateBinaryTree()
{
   std::unique_ptr<Tree<std::string>> tree(new Tree<std::string>("A"));
   tree->GetHead()->
         PrependChild("B")->
         PrependChild("C")->
         PrependChild("D")->
         PrependChild("E")->
         PrependChild("F")->
         PrependChild("G")->
         PrependChild("H");

   const std::vector<std::string> expectedTraversal { "H", "G", "F", "E", "D", "C", "B", "A" };
   std::vector<std::string> setDifference;

   std::set_difference(std::begin(*tree), std::end(*tree), std::begin(expectedTraversal),
                       std::end(expectedTraversal), std::back_inserter(setDifference));

   QVERIFY(setDifference.size() == 0);
}

void TreeTests::PostOrderTraversalOfRightDegenerateBinaryTree()
{
   std::unique_ptr<Tree<std::string>> tree(new Tree<std::string>("A"));
   tree->GetHead()->
         AppendChild("B")->
         AppendChild("C")->
         AppendChild("D")->
         AppendChild("E")->
         AppendChild("F")->
         AppendChild("G")->
         AppendChild("H");

   const std::vector<std::string> expectedTraversal { "H", "G", "F", "E", "D", "C", "B", "A" };
   std::vector<std::string> setDifference;

   std::set_difference(std::begin(*tree), std::end(*tree), std::begin(expectedTraversal),
                       std::end(expectedTraversal), std::back_inserter(setDifference));

   QVERIFY(setDifference.size() == 0);
}

void TreeTests::PostOrderTraversalFromEndToBegin()
{
   std::unique_ptr<Tree<std::string>> tree = CreateSimpleStringBinaryTree();

   const std::vector<std::string> expectedTraversal { "F", "G", "I", "H", "B", "D", "E", "C", "A" };
   int index = 0;

   bool traversalError = false;
   Tree<std::string>::PostOrderIterator itr = std::end(*tree);
   --itr;

   for (; itr != std::begin(*tree); --itr)
   {
      if (*itr != expectedTraversal[index++])
      {
         traversalError = true;
         break;
      }
   }

   // Since begin() actually points to the first node, and not "past" it, like end() would, we need
   // one extra test:
   if (*itr != expectedTraversal[index])
   {
      traversalError = true;
   }

   QVERIFY(traversalError == false);
}

void TreeTests::ReversePostOrderTraversalOfSimpleBinaryTree()
{
   std::unique_ptr<Tree<std::string>> tree = CreateSimpleStringBinaryTree();

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

   QVERIFY(traversalError == false);
}

void TreeTests::PostOrderTraversalForwardsAndBackwards()
{
   std::unique_ptr<Tree<std::string>> tree = CreateSimpleStringBinaryTree();
   Tree<std::string>::PostOrderIterator itr = std::begin(*tree);

   itr++;
   itr++;
   itr++;
   assert(*itr == "D");

   itr--;
   itr--;
   assert(*itr == "C");

   itr++;
   itr++;
   itr++;
   assert(*itr == "B");

   itr--;
   itr--;
   itr--;
   itr--;
   assert(*itr == "A");

   itr++;
   itr++;
   itr++;
   itr++;
   itr++;
   itr++;
   assert(*itr == "I");

   itr--;
   QVERIFY(*itr == "H");
}

void TreeTests::SiblingTraversal()
{
   std::unique_ptr<Tree<std::string>> tree = CreateRootNodeWithManyChildren();
   std::shared_ptr<TreeNode<std::string>> node = tree->GetHead()->GetFirstChild();
   Tree<std::string>::SiblingIterator itr = tree->beginSibling(node);

   const std::vector<std::string> expectedTraversal { "A", "B", "C", "D", "E" };
   int index = 0;

   bool traversalError = false;
   for (auto itr = tree->beginSibling(node); itr != tree->endSibling(node); ++itr)
   {
      if (*itr != expectedTraversal[index++])
      {
         traversalError = true;
         break;
      }
   }

   QVERIFY(traversalError == false);
}

void TreeTests::LeafTraversalOfSimpleBinaryTree()
{
   std::unique_ptr<Tree<std::string>> tree = CreateSimpleStringBinaryTree();

   const std::vector<std::string> expectedTraversal { "A", "C", "E", "H" };
   int index = 0;

   bool traversalError = false;
   for (auto itr = tree->beginLeaf(); itr != tree->endLeaf(); ++itr)
   {
      if (*itr != expectedTraversal[index++])
      {
         traversalError = true;
         break;
      }
   }

   QVERIFY(traversalError == false);
}

QTEST_MAIN(TreeTests)
#include "treetests.moc"
