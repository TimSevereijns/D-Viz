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
      void AddFirstChild(std::shared_ptr<TreeNode<T>> child);

      void PrependChild(std::shared_ptr<TreeNode<T>> child);
      void AppendChild(std::shared_ptr<TreeNode<T>> child);

      std::shared_ptr<TreeNode<T>> m_parent;
      std::shared_ptr<TreeNode<T>> m_firstChild;
      std::shared_ptr<TreeNode<T>> m_lastChild;
      std::shared_ptr<TreeNode<T>> m_previousSibling;
      std::shared_ptr<TreeNode<T>> m_nextSibling;

      unsigned int m_childCount;

      T m_data;

   public:
      explicit TreeNode();
      explicit TreeNode(T data);
      explicit TreeNode(const TreeNode<T>& otherTree);
      ~TreeNode();

      TreeNode<T>& operator=(TreeNode<T> other);

      /**
       * @brief PrependChild        Adds a child node as the first child of the node.
       * @param data                The underlying data to be stored in the node.
       */
      void PrependChild(T data);

      /**
       * @brief AppendChild         Adds a child node as the last child of the node.
       * @param data                The underlying data to be stored in the node.
       */
      void AppendChild(T data);

      /**
       * @brief GetData             Retrieves the data stored in the node.
       * @returns The underlying data stored in the node.
       */
      T GetData() const;

      /**
       * @brief GetParent           Retrieves the parent of the node.
       * @returns A shared_ptr to the parent of the node if it exists; nullptr otherwise.
       */
      std::shared_ptr<TreeNode<T>> GetParent() const;

      /**
       * @brief GetFirstChild       Retrieves the first child of the node.
       * @returns A reference to the first child of the current node.
       */
      TreeNode<T>& GetFirstChild() const;
      TreeNode<T>& GetLastChild() const;
      TreeNode<T>& GetNextSibling() const;
      TreeNode<T>& GetPreviousSibling() const;

      bool HasChildren() const;

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
     m_childCount(0)
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
     m_childCount(0)
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
     m_childCount(otherTree.m_childCount)
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
std::shared_ptr<TreeNode<T>> TreeNode<T>::GetParent() const
{
   return m_parent;
}

template<typename T>
void TreeNode<T>::AddFirstChild(std::shared_ptr<TreeNode<T>> child)
{
   assert(m_childCount == 0);

   //child->m_parent = shared_from_this();

   m_firstChild = child;
   m_lastChild = m_firstChild;

   m_childCount++;
}

template<typename T>
void TreeNode<T>::PrependChild(T data)
{
   const auto newNode = std::make_shared<TreeNode<T>>(data);
   PrependChild(newNode);
}

template<typename T>
void TreeNode<T>::PrependChild(std::shared_ptr<TreeNode<T>> child)
{
   child->m_parent = shared_from_this();

   if (!m_firstChild)
   {
      AddFirstChild(child);
      return;
   }

   assert(m_firstChild);

   m_firstChild->m_previousSibling = child;
   m_firstChild->m_previousSibling->m_nextSibling = m_firstChild;
   m_firstChild = m_firstChild->m_previousSibling;

   m_childCount++;
}

template<typename T>
void TreeNode<T>::AppendChild(T data)
{
   const auto newNode = std::make_shared<TreeNode<T>>(data);
   AppendChild(newNode);
}

template<typename T>
void TreeNode<T>::AppendChild(std::shared_ptr<TreeNode<T>> child)
{
   child->m_parent = shared_from_this();

   if (!m_lastChild)
   {
      AddFirstChild(child);
      return;
   }

   assert(m_lastChild);

   m_lastChild->m_nextSibling = child;
   m_lastChild->m_nextSibling->m_previousSibling = m_lastChild;
   m_lastChild = m_lastChild->m_nextSibling;

   m_childCount++;
}

template<typename T>
T TreeNode<T>::GetData() const
{
   return m_data;
}

template<typename T>
TreeNode<T>& TreeNode<T>::GetFirstChild() const
{
   if (!m_firstChild)
   {
      throw std::range_error("Node does not appear to have a child!");
   }

   return *m_firstChild;
}

template<typename T>
TreeNode<T>& TreeNode<T>::GetLastChild() const
{
   if (!m_lastChild)
   {
      throw std::range_error("Node does not appear to have a child!");
   }

   return *m_lastChild;
}

template<typename T>
TreeNode<T>& TreeNode<T>::GetNextSibling() const
{
   if (!m_nextSibling)
   {
      throw std::range_error("Next sibling does not appear to exist!");
   }

   return *m_nextSibling;
}

template<typename T>
TreeNode<T>& TreeNode<T>::GetPreviousSibling() const
{
   if (!m_previousSibling)
   {
      throw std::range_error("Previous sibling does not appear to exist!");
   }

   return *m_previousSibling;
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
   class Iterator;
   class SiblingIterator;
   class PostOrderIterator;

   public:
      explicit Tree();
      explicit Tree(T data);
      explicit Tree(const Tree<T>& otherTree);
      ~Tree();

      TreeNode<T>& GetHead() const;

      /**
       * @brief The Iterator class
       */
      class Iterator
      {
         public:
            explicit Iterator();
            explicit Iterator(TreeNode<T> node);

            T& operator*() const;
            T* operator->() const;

            SiblingIterator begin() const;
            SiblingIterator end() const;

            bool operator==(const Iterator& iterator) const;
            bool operator!=(const Iterator& iterator) const;

         private:
            std::shared_ptr<TreeNode<T>> m_node;
      };

      /**
       * @brief The SiblingIterator class
       */
      class SiblingIterator : public Iterator
      {
         public:
            explicit SiblingIterator();
            explicit SiblingIterator(TreeNode<T> node);
            explicit SiblingIterator(const SiblingIterator& iterator);
            explicit SiblingIterator(const Iterator& iterator);

            SiblingIterator& operator++();
            SiblingIterator& operator--();

            TreeNode<T>& firstInRange() const;
            TreeNode<T>& lastInRange() const;

         private:
            std::shared_ptr<TreeNode<T>> m_parent;
      };

      /**
       * @brief The PostOrderIterator class
       */
      class PostOrderIterator : public Iterator
      {
         public:
            explicit PostOrderIterator();
            explicit PostOrderIterator(TreeNode<T> node);
            explicit PostOrderIterator(const PostOrderIterator& iterator);
            explicit PostOrderIterator(const Iterator& iterator);

            bool operator==(const PostOrderIterator& iterator) const;
            bool operator!=(const PostOrderIterator& iterator) const;
            PostOrderIterator& operator++();
            PostOrderIterator& operator--();

         private:
            std::shared_ptr<TreeNode<T>> m_parent;
      };

      SiblingIterator begin(const Iterator& iterator) const;
      SiblingIterator end(const Iterator& iterator) const;

      PostOrderIterator begin() const;
      PostOrderIterator end() const;

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
TreeNode<T>& Tree<T>::GetHead() const
{
   return *m_head;
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
   if (iterator.m_node->m_firstChild == nullptr)
   {
      return end(iterator);
   }

   return iterator.m_node->m_firstChild;
}

template<typename T>
typename Tree<T>::PostOrderIterator Tree<T>::end() const
{
   return Tree<T>::PostOrderIterator();
}

/***************************************************************************************************
 * Start of Iterator Class Definitions
 **************************************************************************************************/

template<typename T>
Tree<T>::Iterator::Iterator()
   : m_node(nullptr)
{
}

template<typename T>
T& Tree<T>::Iterator::operator*() const
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

/***************************************************************************************************
 * Start of Sibling Iterator Class Definitions
 **************************************************************************************************/

template<typename T>
Tree<T>::SiblingIterator::SiblingIterator()
   : Iterator(), m_parent(nullptr)
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
   : Iterator(), m_parent(nullptr)
{
}

template<typename T>
typename Tree<T>::PostOrderIterator& Tree<T>::PostOrderIterator::operator++()
{
   assert(m_node);

   // TODO: Add visitation boolean!

   if (!m_node->m_nextSibling)
   {
      m_node = m_node->m_parent;
   }
   else
   {
      m_node = m_node->m_nextSibling;

      while (m_node->m_firstChild)
      {
         m_node = m_node->m_firstChild;
      }
   }

   return *this;
}

template<typename T>
typename Tree<T>::PostOrderIterator& Tree<T>::PostOrderIterator::operator--()
{
   assert(m_node);

   // TODO!

   return *this;
}

#endif // TREE_H
