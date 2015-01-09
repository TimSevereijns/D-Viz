#ifndef TREE_H
#define TREE_H

#include <cassert>
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
      std::shared_ptr<TreeNode<T>> AddFirstChild(std::shared_ptr<TreeNode<T>> child);

      std::shared_ptr<TreeNode<T>> PrependChild(std::shared_ptr<TreeNode<T>> child);
      std::shared_ptr<TreeNode<T>> AppendChild(std::shared_ptr<TreeNode<T>> child);

      std::shared_ptr<TreeNode<T>> m_parent;
      std::shared_ptr<TreeNode<T>> m_firstChild;
      std::shared_ptr<TreeNode<T>> m_lastChild;
      std::shared_ptr<TreeNode<T>> m_previousSibling;
      std::shared_ptr<TreeNode<T>> m_nextSibling;

      unsigned int m_childCount;

      bool m_visited;

      T m_data;

   public:
      explicit TreeNode();
      explicit TreeNode(T data);
      explicit TreeNode(const TreeNode<T>& otherTree);
      ~TreeNode();

      TreeNode<T>& operator=(TreeNode<T> other);

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
       * @returns TODO
       */
      std::shared_ptr<TreeNode<T>> PrependChild(T data);

      /**
       * @brief AppendChild         Adds a child node as the last child of the node.
       * @param data                The underlying data to be stored in the node.
       * @returns TODO
       */
      std::shared_ptr<TreeNode<T>> AppendChild(T data);

      /**
       * @brief GetData             Retrieves the data stored in the node.
       * @returns The underlying data stored in the node.
       */
      T GetData() const;

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
       * @returns The number of children that this node has.
       */
      unsigned int GetChildCount() const;
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
TreeNode<T>::~TreeNode()
{
}

template<typename T>
TreeNode<T>& TreeNode<T>::operator=(TreeNode<T> other)
{
   std::swap(other);
   return *this;
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
std::shared_ptr<TreeNode<T>> TreeNode<T>::AddFirstChild(std::shared_ptr<TreeNode<T>> child)
{
   assert(m_childCount == 0);

   //child->m_parent = shared_from_this();

   m_firstChild = child;
   m_lastChild = m_firstChild;

   m_childCount++;

   return m_firstChild;
}

template<typename T>
std::shared_ptr<TreeNode<T>> TreeNode<T>::PrependChild(T data)
{
   const auto newNode = std::make_shared<TreeNode<T>>(data);
   return PrependChild(newNode);
}

template<typename T>
std::shared_ptr<TreeNode<T>> TreeNode<T>::PrependChild(std::shared_ptr<TreeNode<T>> child)
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
std::shared_ptr<TreeNode<T>> TreeNode<T>::AppendChild(T data)
{
   const auto newNode = std::make_shared<TreeNode<T>>(data);
   return AppendChild(newNode);
}

template<typename T>
std::shared_ptr<TreeNode<T>> TreeNode<T>::AppendChild(std::shared_ptr<TreeNode<T>> child)
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
T TreeNode<T>::GetData() const
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
bool TreeNode<T>::HasChildren() const
{
   return m_childCount > 0;
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

      explicit Tree();
      explicit Tree(T data);
      explicit Tree(const Tree<T>& otherTree);
      ~Tree();

      std::shared_ptr<TreeNode<T>> GetHead() const;

      unsigned int Size() const;

      /**
       * @brief The Iterator class
       *
       * This is the base iterator class that all other iterators (sibling, post-, pre-, in-order)
       * will derive from.
       */
      class Iterator
      {
         public:
            // Typedefs needed for STL compliance:
            typedef T                                    value_type;
            typedef T*                                   pointer;
            typedef T&                                   reference;
            typedef size_t                               size_type;
            typedef ptrdiff_t                            difference_type;
            typedef std::bidirectional_iterator_tag      iterator_category;

            explicit Iterator();
            explicit Iterator(const Iterator& other);
            explicit Iterator(std::shared_ptr<TreeNode<T>> node);
            explicit Iterator(std::shared_ptr<TreeNode<T>> node, std::shared_ptr<TreeNode<T>> head);

            T operator*() const;
            T* operator->() const;

            SiblingIterator begin() const;
            SiblingIterator end() const;

            bool operator==(const Iterator& iterator) const;
            bool operator!=(const Iterator& iterator) const;

            void SetHead(std::shared_ptr<TreeNode<T>> node);

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
            explicit SiblingIterator();
            explicit SiblingIterator(const Iterator& other);
            explicit SiblingIterator(std::shared_ptr<TreeNode<T>> node);

            SiblingIterator& operator++();
            SiblingIterator& operator--();

            TreeNode<T>& firstInRange() const;
            TreeNode<T>& lastInRange() const;
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

            PostOrderIterator operator++(int increment);    // post-fix operator
            PostOrderIterator& operator++();                // pre-fix operator
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

            ReversePostOrderIterator operator++(int increment);    // post-fix operator
            ReversePostOrderIterator& operator++();                // pre-fix operator
            ReversePostOrderIterator& operator--();
      };

      SiblingIterator begin(const Iterator& iterator) const;
      SiblingIterator end(const Iterator& iterator) const;

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
      //ReversePostOrderIterator rbegin() const;

      /**
       * @brief end                 Creates an iterator that points to the beginning of the tree.
       * @returns A post-order iterator.
       */
      //ReversePostOrderIterator rend() const;

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
}

template<typename T>
std::shared_ptr<TreeNode<T>> Tree<T>::GetHead() const
{
   return m_head;
}

template<typename T>
unsigned int Tree<T>::Size() const
{
   const unsigned int theHeadNode = 1;
   return m_head->GetChildCount() + theHeadNode;
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
typename Tree<T>::PostOrderIterator Tree<T>::begin() const
{
   auto iterator = Tree<T>::PostOrderIterator(m_head);
   iterator.SetHead(m_head);
   return ++iterator;
}

template<typename T>
typename Tree<T>::PostOrderIterator Tree<T>::end() const
{
   auto iterator = Tree<T>::PostOrderIterator();
   iterator.SetHead(m_head);
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
     m_head(nullptr)
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
T Tree<T>::Iterator::operator*() const
{
   return m_node->GetData();
}

template<typename T>
T* Tree<T>::Iterator::operator->() const
{
   return &(m_node->GetData());
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

template<typename T>
void Tree<T>::Iterator::SetHead(std::shared_ptr<TreeNode<T>> node)
{
   m_head = node;
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
   : m_node(other.m_node)
{
}

template<typename T>
Tree<T>::SiblingIterator::SiblingIterator(std::shared_ptr<TreeNode<T>> node)
   : Iterator(node)
{
}

template<typename T>
typename Tree<T>::SiblingIterator& Tree<T>::SiblingIterator::operator++()
{
   if (m_node)
   {
      m_node = m_node->m_nextSibling;
   }

   return *this;
}

template<typename T>
typename Tree<T>::SiblingIterator& Tree<T>::SiblingIterator::operator--()
{
   if (m_node)
   {
      m_node = m_node->m_previousSibling;
   }

   return *this;
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

   if (m_node->HasChildren() && !m_node->GetVisited())
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
   else
   {
      m_node->MarkVisited(false);
      //m_haveChildrenBeenVisited = false;

      m_node = m_node->GetParent();

      if (m_node)
      {
         m_node->MarkVisited(true);
         //m_haveChildrenBeenVisited = true;
      }
   }

   return *this;
}

template<typename T>
typename Tree<T>::PostOrderIterator& Tree<T>::PostOrderIterator::operator--()
{
   assert(m_node);

   // When the iterator is at the end(), then the next position should be the head:
   if (!m_node)
   {
      assert(m_head);

      m_node = m_head;
   }
   else if (m_node->GetLastChild())
   {
      m_node = m_node->GetLastChild();
   }
   else if (m_node->GetPreviousSibling())
   {
      m_node->MarkVisited(false);
      m_node = m_node->GetPreviousSibling();
   }
   else if (m_node->GetParent() && m_node->GetParent()->GetPreviousSibling())
   {
      while (m_node->GetParent() && !m_node->GetParent()->GetPreviousSibling())
      {
         m_node = m_node->GetParent();
      }

      m_node->GetParent()->MarkVisited(true);
      m_node = m_node->GetParent()->GetPreviousSibling();
   }
   else // Must be at the end of the traversal, so clean up visitation tracking for the last node:
   {
      m_node->MarkVisited(false);
   }

   return *this;
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
   : m_node(other.m_node)
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
}

template<typename T>
typename Tree<T>::ReversePostOrderIterator& Tree<T>::ReversePostOrderIterator::operator--()
{
   assert(m_node);

   // TODO!

   return *this;
}

#endif // TREE_H
