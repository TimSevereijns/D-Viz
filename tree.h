#ifndef TREE_H
#define TREE_H

#include <cassert>
#include <cstdint>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <utility>

/**
 * The Tree Node class declares the actual nodes of the tree.
 *
 * Each node has a pointer to its parent, its first and last child, as well as its previous and next
 * sibling.
 */
template<typename T>
class TreeNode : public std::enable_shared_from_this<TreeNode<T>>
{
   private:
      std::shared_ptr<TreeNode<T>> AddFirstChild(std::shared_ptr<TreeNode<T>>& child);

      std::shared_ptr<TreeNode<T>> PrependChild(std::shared_ptr<TreeNode<T>>& child);
      std::shared_ptr<TreeNode<T>> AppendChild(std::shared_ptr<TreeNode<T>>& child);

      // TODO: PrependSibling(...)
      // TODO: AppendSibling(...)

      void MergeSort(
         std::shared_ptr<TreeNode<T>>& list,
         const std::function<bool (TreeNode<T>, TreeNode<T>)>& comparator);

      void DivideList(
         std::shared_ptr<TreeNode<T>> head,
         std::shared_ptr<TreeNode<T>>& lhs,
         std::shared_ptr<TreeNode<T>>& rhs);

      std::shared_ptr<TreeNode<T>> MergeSortedHalves(
         std::shared_ptr<TreeNode<T>>& lhs,
         std::shared_ptr<TreeNode<T>>& rhs,
         const std::function<bool (TreeNode<T>, TreeNode<T>)>& comparator);

      std::shared_ptr<TreeNode<T>> m_parent;
      std::shared_ptr<TreeNode<T>> m_firstChild;
      std::shared_ptr<TreeNode<T>> m_lastChild;
      std::shared_ptr<TreeNode<T>> m_previousSibling;
      std::shared_ptr<TreeNode<T>> m_nextSibling;

      T m_data;

      unsigned int m_childCount;

      bool m_visited;

   public:
      explicit TreeNode();
      explicit TreeNode(T data);
      TreeNode(const TreeNode<T>& otherTree);

      TreeNode<T>& operator=(TreeNode<T> other);

      bool operator<(const TreeNode<T>& rhs) const;
      bool operator<=(const TreeNode<T>& rhs) const;
      bool operator>(const TreeNode<T>& rhs) const;
      bool operator>=(const TreeNode<T>& rhs) const;

      T* operator->();
      const T* operator->() const;

      /**
       * @brief GetVisited          Retrieve visitation status of the node
       * @returns True if the node has already been visited.
       */
      bool GetVisited() const;

      /**
       * @brief MarkVisited         Set node visitation status.
       * @param visited             Whether the node should be marked as having been visited.
       */
      void MarkVisited(const bool visited);

      /**
       * @brief PrependChild        Adds a child node as the first child of the node.
       * @param data                The underlying data to be stored in the node.
       * @returns the prepended node.
       */
      std::shared_ptr<TreeNode<T>> PrependChild(T&& data);

      /**
       * @brief AppendChild         Adds a child node as the last child of the node.
       * @param data                The underlying data to be stored in the node.
       * @returns the appended node.
       */
      std::shared_ptr<TreeNode<T>> AppendChild(T&& data);

      /**
       * @brief GetData             Retrieves reference to the data stored in the node.
       * @returns The underlying data stored in the node.
       */
      T& GetData();

      /**
       * @brief GetData             Retrieves const-ref to the data stored in the node.
       * @returns The underlying data stored in the node.
       */
      const T& GetData() const;

      /**
       * @brief GetParent           Retrieves the parent of the node.
       * @returns A shared_ptr to this node's parent, if it exists; nullptr otherwise.
       */
      std::shared_ptr<TreeNode<T>> GetParent() const;

      /**
       * @brief GetFirstChild       Retrieves the first child of the node.
       * @returns A reference to this node's first child.
       */
      std::shared_ptr<TreeNode<T>> GetFirstChild() const;

      /**
       * @brief GetLastChild        Retrieves the last child of the node.
       * @return A reference to this node's last child.
       */
      std::shared_ptr<TreeNode<T>> GetLastChild() const;

      /**
       * @brief GetNextSibling      Retrieves the node that follows the node.
       * @return A reference to this node's next sibling.
       */
      std::shared_ptr<TreeNode<T>> GetNextSibling() const;

      /**
       * @brief GetPreviousSibling  Retrieves the node before the node.
       * @returns A refenence to this node's previous sibling.
       */
      std::shared_ptr<TreeNode<T>> GetPreviousSibling() const;

      /**
       * @brief HasChildren         Indicates whether the node has children.
       * @returns True if this node has children.
       */
      bool HasChildren() const;

      /**
       * @brief GetChildCount       Retrieves the child count of the node.
       * @returns The number of immediate children that this node has.
       */
      unsigned int GetChildCount() const;

      /**
       * @brief CountDescendants      Traverses the tree, counting all descendants.
       * @return The total number of descendant nodes belonging to the node.
       */
      unsigned int CountAllDescendants() const;

      /**
       * @brief SortChildren        Merge sorts the immediate child nodes.
       * @param comparator          The function to be used as the basis for the sorting comparison.
       */
      void SortChildren(const std::function<bool (TreeNode<T>, TreeNode<T>)>& comparator);

      /**
       * @brief RemoveFromTree removes the node from the tree that it is in.
       */
      void RemoveFromTree();
};

/***************************************************************************************************
 * Start of TreeNode<T> Class Definitions
 **************************************************************************************************/

template<typename T>
TreeNode<T>::TreeNode()
   : m_parent(nullptr),
     m_firstChild(nullptr),
     m_lastChild(nullptr),
     m_previousSibling(nullptr),
     m_nextSibling(nullptr),
     m_data(nullptr),
     m_childCount(0),
     m_visited(false)
{
}

template<typename T>
TreeNode<T>::TreeNode(T data)
   : m_parent(nullptr),
     m_firstChild(nullptr),
     m_lastChild(nullptr),
     m_previousSibling(nullptr),
     m_nextSibling(nullptr),
     m_data(data),
     m_childCount(0),
     m_visited(false)
{
}

template<typename T>
TreeNode<T>::TreeNode(const TreeNode<T>& otherTree)
   : m_parent(otherTree.m_parent),
     m_firstChild(otherTree.m_firstChild),
     m_lastChild(otherTree.m_lastChild),
     m_previousSibling(otherTree.m_previousSibling),
     m_nextSibling(otherTree.m_nextSibling),
     m_data(otherTree.m_data),
     m_childCount(otherTree.m_childCount),
     m_visited(otherTree.m_visited)
{
}

template<typename T>
TreeNode<T>& TreeNode<T>::operator=(TreeNode<T> other)
{
   std::swap(this->m_childCount, other.m_childCount);
   std::swap(this->m_data, other.m_data);
   std::swap(this->m_firstChild, other.m_firstChild);
   std::swap(this->m_lastChild, other.m_lastChild);
   std::swap(this->m_nextSibling, other.m_nextSibling);
   std::swap(this->m_parent, other.m_parent);
   std::swap(this->m_previousSibling, other.m_previousSibling);
   std::swap(this->m_visited, other.m_visited);

   return *this;
}

template<typename T>
bool TreeNode<T>::operator<(const TreeNode<T>& rhs) const
{
   return m_data < rhs.GetData();
}

template<typename T>
bool TreeNode<T>::operator<=(const TreeNode<T>& rhs) const
{
   return m_data <= rhs.GetData();
}

template<typename T>
bool TreeNode<T>::operator>(const TreeNode<T>& rhs) const
{
   return m_data > rhs.GetData();
}

template<typename T>
bool TreeNode<T>::operator>=(const TreeNode<T>& rhs) const
{
   return m_data >= rhs.GetData();
}

template<typename T>
bool TreeNode<T>::GetVisited() const
{
   return m_visited;
}

template<typename T>
void TreeNode<T>::MarkVisited(const bool visited)
{
   m_visited = visited;
}

template<typename T>
std::shared_ptr<TreeNode<T>> TreeNode<T>::GetParent() const
{
   return m_parent;
}

template<typename T>
std::shared_ptr<TreeNode<T>> TreeNode<T>::AddFirstChild(std::shared_ptr<TreeNode<T>>& child)
{
   assert(m_childCount == 0);

   m_firstChild = child;
   m_lastChild = m_firstChild;

   m_childCount++;

   return m_firstChild;
}

template<typename T>
std::shared_ptr<TreeNode<T>> TreeNode<T>::PrependChild(T&& data)
{
   const auto newNode = std::make_shared<TreeNode<T>>(std::forward<T>(data));
   return PrependChild(newNode);
}

template<typename T>
std::shared_ptr<TreeNode<T>> TreeNode<T>::PrependChild(std::shared_ptr<TreeNode<T>>& child)
{
   child->m_parent = shared_from_this();

   if (!m_firstChild)
   {
      return AddFirstChild(child);
   }

   assert(m_firstChild);

   m_firstChild->m_previousSibling = child;
   m_firstChild->m_previousSibling->m_nextSibling = m_firstChild;
   m_firstChild = m_firstChild->m_previousSibling;

   m_childCount++;

   return m_firstChild;
}

template<typename T>
std::shared_ptr<TreeNode<T>> TreeNode<T>::AppendChild(T&& data)
{
   auto newNode = std::make_shared<TreeNode<T>>(std::forward<T>(data));
   return AppendChild(newNode);
}

template<typename T>
std::shared_ptr<TreeNode<T>> TreeNode<T>::AppendChild(std::shared_ptr<TreeNode<T>>& child)
{
   child->m_parent = shared_from_this();

   if (!m_lastChild)
   {
      return AddFirstChild(child);
   }

   assert(m_lastChild);

   m_lastChild->m_nextSibling = child;
   m_lastChild->m_nextSibling->m_previousSibling = m_lastChild;
   m_lastChild = m_lastChild->m_nextSibling;

   m_childCount++;

   return m_lastChild;
}

template<typename T>
T* TreeNode<T>::operator->()
{
   return &m_data;
}


template<typename T>
const T* TreeNode<T>::operator->() const
{
   return &m_data;
}

template<typename T>
T& TreeNode<T>::GetData()
{
   return m_data;
}

template<typename T>
const T& TreeNode<T>::GetData() const
{
   return m_data;
}

template<typename T>
std::shared_ptr<TreeNode<T>> TreeNode<T>::GetFirstChild() const
{
   return m_firstChild;
}

template<typename T>
std::shared_ptr<TreeNode<T>> TreeNode<T>::GetLastChild() const
{
   return m_lastChild;
}

template<typename T>
std::shared_ptr<TreeNode<T>> TreeNode<T>::GetNextSibling() const
{
   return m_nextSibling;
}

template<typename T>
std::shared_ptr<TreeNode<T>> TreeNode<T>::GetPreviousSibling() const
{
   return m_previousSibling;
}

template<typename T>
unsigned int TreeNode<T>::GetChildCount() const
{
   return m_childCount;
}

template<typename T>
unsigned int TreeNode<T>::CountAllDescendants() const
{
   return Tree<T>::Size(*this) - 1;
}

template<typename T>
bool TreeNode<T>::HasChildren() const
{
   return m_childCount > 0;
}

template<typename T>
void TreeNode<T>::RemoveFromTree()
{
   // First, remove all references to this node as parent:
   auto& currentChild = m_firstChild;
   while (currentChild)
   {
      currentChild->m_parent = nullptr;
      currentChild = currentChild->m_nextSibling;
   }

   // Now update all sibling and parent relations:
   if (m_previousSibling && m_nextSibling)
   {
      m_previousSibling->m_nextSibling = m_nextSibling;
      m_nextSibling->m_previousSibling = m_previousSibling;
   }
   else if (m_previousSibling && !m_nextSibling)
   {
      m_previousSibling->m_nextSibling = nullptr;

      if (m_parent)
      {
         m_parent->m_lastChild = m_previousSibling;
      }
   }
   else if (m_nextSibling && !m_previousSibling)
   {
      m_nextSibling->m_previousSibling = nullptr;

      if (m_parent)
      {
         m_parent->m_firstChild = m_nextSibling;
      }
   }

   // Nuke references to anything below this node:
   m_firstChild = nullptr;
   m_lastChild = nullptr;

   if (m_parent)
   {
      m_parent->m_childCount--;
   }

#ifndef NDEBUG
//   auto temp = shared_from_this();
//   auto useCount = temp.use_count();
//   assert(useCount == 2);
//   temp.reset();
#endif
}

template<typename T>
void TreeNode<T>::SortChildren(const std::function<bool (TreeNode<T>, TreeNode<T>)>& comparator)
{
   MergeSort(m_firstChild, comparator);
}

template<typename T>
void TreeNode<T>::MergeSort(std::shared_ptr<TreeNode<T>>& list,
   const std::function<bool (TreeNode<T>, TreeNode<T>)>& comparator)
{
   if (!list || !list->m_nextSibling)
   {
      return;
   }

   std::shared_ptr<TreeNode<T>> head = list;
   std::shared_ptr<TreeNode<T>> lhs = nullptr;
   std::shared_ptr<TreeNode<T>> rhs = nullptr;

   DivideList(head, lhs, rhs);

   assert(lhs);
   assert(rhs);

   MergeSort(lhs, comparator);
   MergeSort(rhs, comparator);

   list = MergeSortedHalves(lhs, rhs, comparator);
}

template<typename T>
void TreeNode<T>::DivideList(std::shared_ptr<TreeNode<T>> head,
   std::shared_ptr<TreeNode<T>>& lhs, std::shared_ptr<TreeNode<T>>& rhs)
{
   if (!head || !head->m_nextSibling)
   {
      return;
   }

   std::shared_ptr<TreeNode<T>> tortoise = head;
   std::shared_ptr<TreeNode<T>> hare = head->m_nextSibling;

   while (hare)
   {
      hare = hare->m_nextSibling;
      if (hare)
      {
         tortoise = tortoise->m_nextSibling;
         hare = hare->m_nextSibling;
      }
   }

   lhs = head;
   rhs = tortoise->m_nextSibling;

   tortoise->m_nextSibling = nullptr;
}

template<typename T>
std::shared_ptr<TreeNode<T>> TreeNode<T>::MergeSortedHalves(std::shared_ptr<TreeNode<T>>& lhs,
   std::shared_ptr<TreeNode<T>>& rhs,
   const std::function<bool (TreeNode<T>, TreeNode<T>)>& comparator)
{
   std::shared_ptr<TreeNode<T>> result = nullptr;
   if (comparator(*lhs.get(), *rhs.get()))
   {
      result = std::shared_ptr<TreeNode<T>>(lhs);
      lhs = lhs->m_nextSibling;
   }
   else
   {
      result = std::shared_ptr<TreeNode<T>>(rhs);
      rhs = rhs->m_nextSibling;
   }

   std::shared_ptr<TreeNode<T>> tail = result;

   while (lhs && rhs)
   {
      if (comparator(*lhs.get(), *rhs.get()))
      {
         tail->m_nextSibling = std::shared_ptr<TreeNode<T>>(lhs);
         tail = tail->m_nextSibling;

         lhs = lhs->m_nextSibling;

         if (lhs)
         {
            lhs->m_previousSibling = nullptr;
         }
      }
      else
      {
         tail->m_nextSibling = std::shared_ptr<TreeNode<T>>(rhs);
         tail = tail->m_nextSibling;

         rhs = rhs->m_nextSibling;

         if (rhs)
         {
            rhs->m_previousSibling = nullptr;
         }
      }
   }

   while (lhs)
   {
      tail->m_nextSibling = std::shared_ptr<TreeNode<T>>(lhs);
      tail = tail->m_nextSibling;

      lhs = lhs->m_nextSibling;

      if (lhs)
      {
         lhs->m_previousSibling = nullptr;
      }
   }

   while (rhs)
   {
      tail->m_nextSibling = std::shared_ptr<TreeNode<T>>(rhs);
      tail = tail->m_nextSibling;

      rhs = rhs->m_nextSibling;

      if (rhs)
      {
         rhs->m_previousSibling = nullptr;
      }
   }

   return result;
}

/**
 * The Tree class declares a basic n-ary tree, built on top of templatized TreeNode nodes.
 *
 * Each tree consists of a simple head TreeNode and nothing else.
 */
template<typename T>
class Tree
{
   public:
      class Iterator;
      class SiblingIterator;
      class PostOrderIterator;
      class ReversePostOrderIterator;

      explicit Tree();
      explicit Tree(T data);
      explicit Tree(const Tree<T>& otherTree);
      ~Tree();

      /**
       * @brief GetHead
       * @return
       */
      std::shared_ptr<TreeNode<T>> GetHead() const;

      /**
       * @brief SetHead             Sets the head.
       * @param head                The data to be placed in the head node.
       */
      void SetHead(const T head);

      /**
       * @brief CountLeafNodes      Traverses the tree, counting all leaf nodes.
       * @return The total number of leaf nodes belonging to the node.
       */
      unsigned int CountLeafNodes() const;

      /**
       * @brief Size                Run-time is linear in the size of the entire tree.
       * @returns The total number of nodes in the tree (both leaf and non-leaf).
       */
      unsigned int Size() const;

      /**
       * @brief Size                Run-time is linear in the size of the sub-tree.
       * @param node                The node from which to compute the size of the sub-tree.
       * @returns The total number of nodes (both leaf and branching) in the tree, starting at the
       * passed in node.
       */
      static unsigned int Size(const TreeNode<T>& node);

      /**
       * @brief Depth               Computes the depth of the node in the tree by counting its
       *                            ancestors. The root is at depth zero.
       * @param node                The node whose depth is to be assertained.
       * @returns The zero-indexed depth of the node in its tree.
       */
      static unsigned int Depth(TreeNode<T> node);

      /**
       * @brief Print               Prints the tree starting and ending at the node in question,
       *                            using a pre-order traversal.
       * @param node                The starting node.
       * @param printer             A function (or lambda) that interprets the node's data and
       *                            returns the data to be printed as a std::wstring.
       */
      static void Print(TreeNode<T>& node, std::function<std::wstring (const T&)> printer);

      /**
       * @brief The Iterator class
       *
       * This is the base iterator class that all other iterators (sibling, post-, pre-, in-order)
       * will derive from.
       */
      class Iterator
      {
         public:
            // So that m_head can be set without a public setter.
            friend class Tree<T>;

            // Typedefs needed for STL compliance:
            typedef T                                    value_type;
            typedef T*                                   pointer;
            typedef T&                                   reference;
            typedef std::size_t                          size_type;
            typedef std::ptrdiff_t                       difference_type;
            typedef std::bidirectional_iterator_tag      iterator_category;

            explicit Iterator();
            explicit Iterator(const Iterator& other);
            explicit Iterator(std::shared_ptr<TreeNode<T>> node);
            explicit Iterator(std::shared_ptr<TreeNode<T>> node, std::shared_ptr<TreeNode<T>> head);

            TreeNode<T>& operator*();
            const TreeNode<T>& operator*() const;

            TreeNode<T>* operator->();
            const TreeNode<T>* operator->() const;

            SiblingIterator begin() const;
            SiblingIterator end() const;

            bool operator==(const Iterator& iterator) const;
            bool operator!=(const Iterator& iterator) const;

         protected:
            std::shared_ptr<TreeNode<T>> m_node;
            std::shared_ptr<TreeNode<T>> m_head;
      };

      /**
       * @brief The SiblingIterator class
       */
      class SiblingIterator : public Iterator
      {
         public:
            // So that m_parent be set without a public setter:
            friend class Tree<T>;

            explicit SiblingIterator();
            explicit SiblingIterator(const Iterator& other);
            explicit SiblingIterator(std::shared_ptr<TreeNode<T>> node);

            SiblingIterator operator++(int);             // post-fix operator; do not return ref!
            SiblingIterator& operator++();               // pre-fix operator
            SiblingIterator operator--(int);
            SiblingIterator& operator--();

         private:
            std::shared_ptr<TreeNode<T>> m_parent;
      };

      /**
       * @brief The PreOrderIterator class
       */
      class PreOrderIterator : public Iterator
      {
         public:
            explicit PreOrderIterator();
            explicit PreOrderIterator(const Iterator& other);
            explicit PreOrderIterator(std::shared_ptr<TreeNode<T>> node);
            explicit PreOrderIterator(std::shared_ptr<TreeNode<T>> node,
                                      std::shared_ptr<TreeNode<T>> head);

            PreOrderIterator operator++(int);           // post-fix operator; do not return ref!
            PreOrderIterator& operator++();             // pre-fix operator
            PreOrderIterator operator--(int);
            PreOrderIterator& operator--();
      };

      /**
       * @brief The PostOrderIterator class
       */
      class PostOrderIterator : public Iterator
      {
         public:
            explicit PostOrderIterator();
            explicit PostOrderIterator(const Iterator& other);
            explicit PostOrderIterator(std::shared_ptr<TreeNode<T>> node);
            explicit PostOrderIterator(std::shared_ptr<TreeNode<T>> node,
                                       std::shared_ptr<TreeNode<T>> head);

            PostOrderIterator operator++(int);           // post-fix operator; do not return ref!
            PostOrderIterator& operator++();             // pre-fix operator
            PostOrderIterator operator--(int);
            PostOrderIterator& operator--();

         private:
            bool m_haveChildrenBeenVisited;
      };

      /**
       * @brief The PostOrderIterator class
       */
      class ReversePostOrderIterator : public Iterator
      {
         public:
            explicit ReversePostOrderIterator();
            explicit ReversePostOrderIterator(const Iterator& other);
            explicit ReversePostOrderIterator(std::shared_ptr<TreeNode<T>> node);

            ReversePostOrderIterator operator++(int);    // post-fix; do not return ref!
            ReversePostOrderIterator& operator++();      // pre-fix operator
            ReversePostOrderIterator operator--(int);
            ReversePostOrderIterator& operator--();
      };

      /**
       * @brief The LeafIterator class
       */
      class LeafIterator : public Iterator
      {
         public:
            explicit LeafIterator();
            explicit LeafIterator(const Iterator& other);
            explicit LeafIterator(std::shared_ptr<TreeNode<T>> node);

            LeafIterator operator++(int);                // post-fix operator; do not return ref!
            LeafIterator& operator++();                  // pre-fix operator
            LeafIterator operator--(int);
            LeafIterator& operator--();
      };

      SiblingIterator begin(const Iterator& iterator) const;
      SiblingIterator end(const Iterator& iterator) const;

      /**
       * @brief beginSibling        Creates a sibling iterator starting at the specified node.
       * @param node                The starting node.
       * @returns An iterator that advances over the siblings of the node.
       */
      SiblingIterator beginSibling(const std::shared_ptr<TreeNode<T>> node) const;

      /**
       * @brief endSibling          Creates a sibling iterator that points past the end of the last
       *                            sibling.
       * @param node                Any node that is a sibling of the target range.
       * @returns An iterator past the end of the last sibling.
       */
      SiblingIterator endSibling(const std::shared_ptr<TreeNode<T>> node) const;

      /**
       * @brief beginPreOrder
       * @return A pre-order iterator that will iterate over all nodes in the tree.
       */
      PreOrderIterator beginPreOrder() const;

      /**
       * @brief endPreOrder
       * @return A pre-order iterator pointing "past" the end of the tree.
       */
      PreOrderIterator endPreOrder() const;

      /**
       * @brief begin               Creates an iterator pointing to the head of the tree.
       * @returns A post-order iterator that will iterator over all nodes in the tree.
       */
      PostOrderIterator begin() const;

      /**
       * @brief end                 Creates an iterator that points to the end of the tree.
       * @returns A post-order iterator.
       */
      PostOrderIterator end() const;

      /**
       * @brief end                 Creates an iterator that points to the end of the tree.
       * @returns A post-order iterator.
       */
      ReversePostOrderIterator rbegin() const;

      /**
       * @brief end                 Creates an iterator that points to the beginning of the tree.
       * @returns A post-order iterator.
       */
      ReversePostOrderIterator rend() const;

      /**
       * @brief beginLeaf           Creates a leaf iterator that starts at the left-most leaf in
       *                            the tree.
       * @returns An iterator to the first leaf node.
       */
      LeafIterator beginLeaf() const;

      /**
       * @brief endLeaf             Creates a leaf iterator that points to nullptr.
       * @return An iterator past the end of the tree.
       */
      LeafIterator endLeaf() const;

   private:
      std::shared_ptr<TreeNode<T>> m_head;
};

/***************************************************************************************************
 * Start of Tree<T> Class Definitions
 **************************************************************************************************/

template<typename T>
Tree<T>::Tree()
   : m_head(new TreeNode<T>())
{
}

template<typename T>
Tree<T>::Tree(T data)
   : m_head(new TreeNode<T>(data))
{
}

template<typename T>
Tree<T>::Tree(const Tree<T>& otherTree)
   : m_head(otherTree.m_head)
{
}

template<typename T>
Tree<T>::~Tree()
{
   std::cout << "The Tree is dead..." << std::endl;
}

template<typename T>
std::shared_ptr<TreeNode<T>> Tree<T>::GetHead() const
{
   return m_head;
}

template<typename T>
void Tree<T>::SetHead(const T data)
{
   auto head = std::shared_ptr<TreeNode<T>>(new TreeNode<T>(data));
   m_head = head;
}

template<typename T>
unsigned int Tree<T>::CountLeafNodes() const
{
   unsigned int count = 0;
   for (auto itr = beginLeaf(); itr != endLeaf(); ++itr)
   {
      count++;
   }

   return count;
}

template<typename T>
unsigned int Tree<T>::Size() const
{
   return std::count_if(std::begin(*this), std::end(*this),
      [](const TreeNode<T>&)
   {
      return true;
   });
}

template<typename T>
unsigned int Tree<T>::Size(const TreeNode<T>& node)
{
   unsigned int count = 0;

   Tree<T>::PostOrderIterator itr = Tree<T>::PostOrderIterator(std::make_shared<TreeNode<T>>(node));
   for (++itr; &*itr != &node; ++itr)
   {
      count++;
   }

   return count;
}

template<typename T>
unsigned int Tree<T>::Depth(TreeNode<T> node)
{
   unsigned int depth = 0;

   TreeNode<T>* nodePtr = &node;
   while (nodePtr->GetParent())
   {
      ++depth;
      nodePtr = &*nodePtr->GetParent();
   }

   return depth;
}

template<typename T>
void Tree<T>::Print(TreeNode<T>& node, std::function<std::wstring (const T&)> printer)
{
   Tree<T>::PreOrderIterator itr = Tree<T>::PreOrderIterator(std::make_shared<TreeNode<T>>(node));
   while (&*itr != &node)
   {
      if (&*itr == nullptr)
      {
         break;
      }

      const auto depth = Tree<T>::Depth(*itr);
      const auto tabSize = 2;
      const std::wstring padding((depth * tabSize), ' ');

      std::wcout << padding << printer(itr->GetData()) << std::endl;

      ++itr;
   }
}

template<typename T>
typename Tree<T>::SiblingIterator Tree<T>::begin(const typename Tree<T>::Iterator& iterator) const
{
   if (iterator.m_node->m_firstChild == nullptr)
   {
      return end(iterator);
   }

   return iterator.m_node->m_firstChild;
}

template<typename T>
typename Tree<T>::SiblingIterator Tree<T>::end(const typename Tree<T>::Iterator& iterator) const
{
   Tree<T>::SiblingIterator siblingIterator(nullptr);
   siblingIterator.m_parent = iterator.m_node;

   return siblingIterator;
}

template<typename T>
typename Tree<T>::SiblingIterator Tree<T>::beginSibling(const std::shared_ptr<TreeNode<T>> node) const
{
   return Tree<T>::SiblingIterator(node);
}

template<typename T>
typename Tree<T>::SiblingIterator Tree<T>::endSibling(const std::shared_ptr<TreeNode<T>> node) const
{
   Tree<T>::SiblingIterator siblingIterator(nullptr);
   siblingIterator.m_parent = node->GetParent();

   return siblingIterator;
}

template<typename T>
typename Tree<T>::PreOrderIterator Tree<T>::beginPreOrder() const
{
   Tree<T>::PreOrderIterator iterator = Tree<T>::PreOrderIterator(m_head);
   iterator.m_head = m_head;

   return iterator;
}

template<typename T>
typename Tree<T>::PreOrderIterator Tree<T>::endPreOrder() const
{
   auto iterator = Tree<T>::PreOrderIterator();
   iterator.m_head = m_head;

   return iterator;
}

template<typename T>
typename Tree<T>::PostOrderIterator Tree<T>::begin() const
{
   Tree<T>::PostOrderIterator iterator = Tree<T>::PostOrderIterator(m_head);
   iterator.m_head = m_head;

   return ++iterator;
}

template<typename T>
typename Tree<T>::PostOrderIterator Tree<T>::end() const
{
   auto iterator = Tree<T>::PostOrderIterator();
   iterator.m_head = m_head;

   return iterator;
}

template<typename T>
typename Tree<T>::ReversePostOrderIterator Tree<T>::rbegin() const
{
   auto iterator = Tree<T>::ReversePostOrderIterator(m_head);
   iterator.m_head = m_head;

   return iterator;
}

template<typename T>
typename Tree<T>::ReversePostOrderIterator Tree<T>::rend() const
{
   auto iterator = Tree<T>::ReversePostOrderIterator();
   iterator.m_head = m_head;

   return iterator;
}

template<typename T>
typename Tree<T>::LeafIterator Tree<T>::beginLeaf() const
{
   auto iterator = Tree<T>::LeafIterator(m_head);
   iterator.m_head = m_head;

   return ++iterator;
}

template<typename T>
typename Tree<T>::LeafIterator Tree<T>::endLeaf() const
{
   auto iterator = Tree<T>::LeafIterator();
   iterator.m_head = m_head;

   return iterator;
}

/***************************************************************************************************
 * Start of Iterator Class Definitions
 **************************************************************************************************/

template<typename T>
Tree<T>::Iterator::Iterator()
   : m_node(nullptr),
     m_head(nullptr)
{
}

template<typename T>
Tree<T>::Iterator::Iterator(const Iterator& other)
   : m_node(other.m_node),
     m_head(other.m_head)
{
}

template<typename T>
Tree<T>::Iterator::Iterator(std::shared_ptr<TreeNode<T>> node)
   : m_node(node),
     m_head(nullptr)
{
}

template<typename T>
Tree<T>::Iterator::Iterator(std::shared_ptr<TreeNode<T>> node, std::shared_ptr<TreeNode<T>> head)
   : m_node(node),
     m_head(head)
{
}

template<typename T>
TreeNode<T>& Tree<T>::Iterator::operator*()
{
   return *m_node;
}


template<typename T>
const TreeNode<T>& Tree<T>::Iterator::operator*() const
{
   return *m_node;
}

template<typename T>
TreeNode<T>* Tree<T>::Iterator::operator->()
{
   return &(*m_node);
}

template<typename T>
const TreeNode<T>* Tree<T>::Iterator::operator->() const
{
   return &(*m_node);
}

template<typename T>
bool Tree<T>::Iterator::operator==(const Iterator& other) const
{
   return m_node == other.m_node;
}

template<typename T>
bool Tree<T>::Iterator::operator!=(const Iterator& other) const
{
   return m_node != other.m_node;
}

template<typename T>
typename Tree<T>::SiblingIterator Tree<T>::Iterator::begin() const
{
   if (m_node && !m_node->m_firstChild)
   {
      return end();
   }

   Tree<T>::SiblingIterator iterator(m_node->m_firstChild);
   iterator.m_parent = m_node;

   return iterator;
}

template<typename T>
typename Tree<T>::SiblingIterator Tree<T>::Iterator::end() const
{
   Tree<T>::SiblingIterator iterator(nullptr);
   iterator.m_parent = m_node;

   return iterator;
}

/***************************************************************************************************
 * Start of Sibling Iterator Class Definitions
 **************************************************************************************************/

template<typename T>
Tree<T>::SiblingIterator::SiblingIterator()
   : Iterator()
{
}

template<typename T>
Tree<T>::SiblingIterator::SiblingIterator(const Iterator& other)
   : m_node(other.m_node),
     m_head(other.m_head)
{
}

template<typename T>
Tree<T>::SiblingIterator::SiblingIterator(std::shared_ptr<TreeNode<T>> node)
   : Iterator(node)
{
}

template<typename T>
typename Tree<T>::SiblingIterator Tree<T>::SiblingIterator::operator++(int)
{
   auto result = *this;
   ++(*this);

   return result;
}

template<typename T>
typename Tree<T>::SiblingIterator& Tree<T>::SiblingIterator::operator++()
{
   if (m_node)
   {
      m_node = m_node->GetNextSibling();
   }

   return *this;
}

template<typename T>
typename Tree<T>::SiblingIterator Tree<T>::SiblingIterator::operator--(int)
{
   auto result = *this;
   --(*this);

   return result;
}

template<typename T>
typename Tree<T>::SiblingIterator& Tree<T>::SiblingIterator::operator--()
{
   if (!m_node)
   {
      if (m_parent)
      {
         m_node = m_parent->GetLastChild();
      }
      else
      {
         // If no parent exists, then the node in question must be the one "past" the head,
         // so decrementing from there should return the head node:
         m_node = m_head;
      }
   }
   else
   {
      m_node = m_node->m_previousSibling;
   }

   return *this;
}

/***************************************************************************************************
 * Start of Pre-Order Iterator Class Definitions
 **************************************************************************************************/

template<typename T>
Tree<T>::PreOrderIterator::PreOrderIterator()
   : Iterator()
{
}

template<typename T>
Tree<T>::PreOrderIterator::PreOrderIterator(const Iterator& other)
   : m_node(other.m_node),
     m_head(other.m_head)
{
}

template<typename T>
Tree<T>::PreOrderIterator::PreOrderIterator(std::shared_ptr<TreeNode<T>> node)
   : Iterator(node)
{
}

template<typename T>
Tree<T>::PreOrderIterator::PreOrderIterator(std::shared_ptr<TreeNode<T>> node,
                                            std::shared_ptr<TreeNode<T>> head)
   : Iterator(node, head)
{
}

template<typename T>
typename Tree<T>::PreOrderIterator Tree<T>::PreOrderIterator::operator++(int)
{
   auto result = *this;
   ++(*this);

   return result;
}

template<typename T>
typename Tree<T>::PreOrderIterator& Tree<T>::PreOrderIterator::operator++()
{
   assert(m_node);

   if (m_node->HasChildren())
   {
      m_node = m_node->GetFirstChild();
   }
   else if (m_node->GetNextSibling())
   {
      m_node = m_node->GetNextSibling();
   }
   else
   {
      while (m_node->GetParent() && !m_node->GetParent()->GetNextSibling())
      {
         m_node = m_node->GetParent();
      }

      if (m_node->GetParent())
      {
         m_node = m_node->GetParent()->GetNextSibling();
      }
      else
      {
         m_node = nullptr;
      }
   }

   return *this;
}

template<typename T>
typename Tree<T>::PreOrderIterator& Tree<T>::PreOrderIterator::operator--()
{
   if (!m_node)
   {
      m_node = m_head;

      while (m_node->GetLastChild())
      {
         m_node = m_node->GetLastChild();
      }
   }
   else if (m_node->GetPreviousSibling())
   {
      m_node = m_node->GetPreviousSibling();

      while (m_node->GetLastChild())
      {
         m_node = m_node->GetLastChild();
      }
   }
   else if (m_node->GetParent())
   {
      m_node = m_node->GetParent();
   }
   else
   {
      m_node = nullptr;
   }

   return *this;
}

template<typename T>
typename Tree<T>::PreOrderIterator Tree<T>::PreOrderIterator::operator--(int)
{
   auto result = *this;
   --(*this);

   return result;
}

/***************************************************************************************************
 * Start of Post-Order Iterator Class Definitions
 **************************************************************************************************/

template<typename T>
Tree<T>::PostOrderIterator::PostOrderIterator()
   : Iterator(),
     m_haveChildrenBeenVisited(false)
{
}

template<typename T>
Tree<T>::PostOrderIterator::PostOrderIterator(const Iterator& other)
   : m_node(other.m_node),
     m_head(other.m_head),
     m_haveChildrenBeenVisited(false)
{
}

template<typename T>
Tree<T>::PostOrderIterator::PostOrderIterator(std::shared_ptr<TreeNode<T>> node)
   : Iterator(node),
     m_haveChildrenBeenVisited(false)
{
}

template<typename T>
Tree<T>::PostOrderIterator::PostOrderIterator(std::shared_ptr<TreeNode<T>> node,
                                              std::shared_ptr<TreeNode<T>> head)
   : Iterator(node, head),
     m_haveChildrenBeenVisited(false)
{
}

template<typename T>
typename Tree<T>::PostOrderIterator Tree<T>::PostOrderIterator::operator++(int)
{
   auto result = *this;
   ++(*this);

   return result;
}

template<typename T>
typename Tree<T>::PostOrderIterator& Tree<T>::PostOrderIterator::operator++()
{
   assert(m_node);

   if (m_node->HasChildren() && !m_haveChildrenBeenVisited)
   {
      while (m_node->GetFirstChild())
      {
         m_node = m_node->GetFirstChild();
      }
   }
   else if (m_node->GetNextSibling())
   {
      m_node = m_node->GetNextSibling();

      while (m_node->HasChildren())
      {
         m_node = m_node->GetFirstChild();
      }
   }
   else
   {
      m_haveChildrenBeenVisited = true;

      m_node = m_node->GetParent();
   }

   return *this;
}

template<typename T>
typename Tree<T>::PostOrderIterator& Tree<T>::PostOrderIterator::operator--()
{
   // When the iterator is at the end(), then the next position should be the head:
   if (!m_node)
   {
      assert(m_head);

      m_node = m_head;
   }
   else if (m_node->HasChildren())
   {
      m_node = m_node->GetLastChild();
   }
   else if (m_node->GetPreviousSibling())
   {
      m_node = m_node->GetPreviousSibling();
   }
   else if (m_node->GetParent())
   {
      while (m_node->GetParent() && !m_node->GetParent()->GetPreviousSibling())
      {
         m_node = m_node->GetParent();
      }

      m_node = m_node->GetParent();

      if (m_node)
      {
         m_node = m_node->GetPreviousSibling();
      }
   }

   return *this;
}

template<typename T>
typename Tree<T>::PostOrderIterator Tree<T>::PostOrderIterator::operator--(int)
{
   auto result = *this;
   --(*this);

   return result;
}

/***************************************************************************************************
 * Start of Reverse Post-Order Iterator Class Definitions
 **************************************************************************************************/

template<typename T>
Tree<T>::ReversePostOrderIterator::ReversePostOrderIterator()
   : Iterator()
{
}

template<typename T>
Tree<T>::ReversePostOrderIterator::ReversePostOrderIterator(const Iterator& other)
   : m_node(other.m_node),
     m_head(other.m_head)
{
}

template<typename T>
Tree<T>::ReversePostOrderIterator::ReversePostOrderIterator(std::shared_ptr<TreeNode<T>> node)
   : Iterator(node)
{
}

template<typename T>
typename Tree<T>::ReversePostOrderIterator Tree<T>::ReversePostOrderIterator::operator++(int)
{
   auto result = *this;
   ++(*this);

   return result;
}

template<typename T>
typename Tree<T>::ReversePostOrderIterator& Tree<T>::ReversePostOrderIterator::operator++()
{
   // When the iterator is at the end(), then the next position should be the head:
   if (!m_node)
   {
      assert(m_head);

      m_node = m_head;
   }
   else if (m_node->HasChildren())
   {
      m_node = m_node->GetLastChild();
   }
   else if (m_node->GetPreviousSibling())
   {
      m_node = m_node->GetPreviousSibling();
   }
   else if (m_node->GetParent())
   {
      while (m_node->GetParent() && !m_node->GetParent()->GetPreviousSibling())
      {
         m_node = m_node->GetParent();
      }

      m_node = m_node->GetParent();

      if (m_node)
      {
         m_node = m_node->GetPreviousSibling();
      }
   }

   return *this;
}

template<typename T>
typename Tree<T>::ReversePostOrderIterator Tree<T>::ReversePostOrderIterator::operator--(int)
{
   auto result = *this;
   --(*this);

   return result;
}

template<typename T>
typename Tree<T>::ReversePostOrderIterator& Tree<T>::ReversePostOrderIterator::operator--()
{
   assert(m_node);

   if (m_node->HasChildren())
   {
      while (m_node->GetFirstChild())
      {
         m_node = m_node->GetFirstChild();
      }
   }
   else if (m_node->GetNextSibling())
   {
      m_node = m_node->GetNextSibling();

      while (m_node->HasChildren())
      {
         m_node = m_node->GetFirstChild();
      }
   }
   else
   {
      m_node = m_node->GetParent();
   }

   return *this;
}

/***************************************************************************************************
 * Start of Leaf Iterator Class Definitions
 **************************************************************************************************/

template<typename T>
Tree<T>::LeafIterator::LeafIterator()
   : Iterator()
{
}

template<typename T>
Tree<T>::LeafIterator::LeafIterator(const Iterator& other)
   : m_node(other.m_node),
     m_head(other.m_head)
{
}

template<typename T>
Tree<T>::LeafIterator::LeafIterator(std::shared_ptr<TreeNode<T>> node)
   : Iterator(node)
{
}

template<typename T>
typename Tree<T>::LeafIterator Tree<T>::LeafIterator::operator++(int)
{
   auto result = *this;
   ++(*this);

   return result;
}

template<typename T>
typename Tree<T>::LeafIterator& Tree<T>::LeafIterator::operator++()
{
   assert(m_node);

   if (m_node->HasChildren())
   {
      while (m_node->GetFirstChild())
      {
         m_node = m_node->GetFirstChild();
      }
   }
   else if (m_node->GetNextSibling())
   {
      m_node = m_node->GetNextSibling();

      while (m_node->GetFirstChild())
      {
         m_node = m_node->GetFirstChild();
      }
   }
   else if (m_node->GetParent())
   {
      while (m_node->GetParent() && !m_node->GetParent()->GetNextSibling())
      {
         m_node = m_node->GetParent();
      }

      if (m_node->GetParent())
      {
         m_node = m_node->GetParent()->GetNextSibling();

         while (m_node->HasChildren())
         {
            m_node = m_node->GetFirstChild();
         }

         return *this;
      }

      // Otherwise, the traversal is at the end:
      m_node = nullptr;
   }

   return *this;
}

template<typename T>
typename Tree<T>::LeafIterator Tree<T>::LeafIterator::operator--(int)
{
   auto result = *this;
   --(*this);

   return result;
}

template<typename T>
typename Tree<T>::LeafIterator& Tree<T>::LeafIterator::operator--()
{
   if (!m_node)
   {
      m_node = m_head;
   }

   if (m_node->HasChildren())
   {
      while (m_node->GetLastChild())
      {
         m_node = m_node->GetLastChild();
      }
   }
   else if (m_node->GetPreviousSibling())
   {
      m_node = m_node->GetPreviousSibling();

      while (m_node->GetLastChild())
      {
         m_node->GetLastChild();
      }
   }
   else if (m_node->GetParent())
   {
      while (m_node->GetParent() && !m_node->GetParent()->GetPreviousSibling())
      {
         m_node = m_node->GetParent();
      }

      if (m_node->GetParent())
      {
         m_node = m_node->GetParent()->GetPreviousSibling();

         while (m_node->HasChildren())
         {
            m_node = m_node->GetLastChild();
         }

         return *this;
      }

      // Otherwise, the traversal is at the end:
      m_node = nullptr;
   }

   return *this;
}

#endif // TREE_H
