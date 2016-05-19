/**
* The MIT License (MIT)
*
* Copyright (c) 2016 Tim Severeijns
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#pragma once

#include <cassert>
#include <functional>
#include <iterator>

template<typename DataType> class Tree;
template<typename DataType> class TreeNode;

/**
* @returns True if the data encapsulated in the left-hand side TreeNode is less than
* the data encapsulated in the right-hand side TreeNode.
*/
template<typename DataType>
constexpr inline bool operator<(const TreeNode<DataType>& lhs, const TreeNode<DataType>& rhs)
{
   return lhs.GetData() < rhs.GetData();
}

/**
* @returns True if the data encapsulated in the left-hand side TreeNode is less than
* or equal to the data encapsulated in the right-hand side TreeNode.
*/
template<typename DataType>
constexpr inline bool operator<=(const TreeNode<DataType>& lhs, const TreeNode<DataType>& rhs)
{
   return !(lhs.GetData() > rhs.GetData());
}

/**
* @returns True if the data encapsulated in the left-hand side TreeNode is greater than
* the data encapsulated in the right-hand side TreeNode.
*/
template<typename DataType>
constexpr inline bool operator>(const TreeNode<DataType>& lhs, const TreeNode<DataType>& rhs)
{
   return rhs.GetData() < lhs.GetData();
}

/**
* @returns True if the data encapsulated in the left-hand side TreeNode is greater than
* or equal to the data encapsulated in the right-hand side TreeNode.
*/
template<typename DataType>
constexpr inline bool operator>=(const TreeNode<DataType>& lhs, const TreeNode<DataType>& rhs)
{
   return !(lhs.GetData() < rhs.GetData());
}

/**
* @returns True if the data encapsulated in the left-hand side TreeNode is equal to
* the data encapsulated in the right-hand side TreeNode.
*/
template<typename DataType>
constexpr inline bool operator==(const TreeNode<DataType>& lhs, const TreeNode<DataType>& rhs)
{
   return lhs.GetData() == rhs.GetData();
}

/**
* @returns True if the data encapsulated in the left-hand side TreeNode is not equal
* to the data encapsulated in the right-hand side TreeNode.
*/
template<typename DataType>
constexpr inline bool operator!=(const TreeNode<DataType>& lhs, const TreeNode<DataType>& rhs)
{
   return !(lhs.GetData() == rhs.GetData());
}

/**
* The TreeNode class represents the nodes that make up the Tree.
*
* Each node has a pointer to its parent, its first and last child, its previous and next
* sibling, and, of course, to the data it encapsulates.
*/
template<typename DataType>
class TreeNode
{
public:
   typedef DataType           value_type;
   typedef DataType&          reference;
   typedef const DataType&    const_reference;

   /**
   * @brief TreeNode default constructs a new TreeNode. All outgoing links from this new node will
   * initialized to a nullptr.
   */
   constexpr TreeNode() noexcept = default;

   /**
   * @brief TreeNode constructs a new TreeNode encapsulating the specified data. All outgoing links
   * from the node will be initialized to nullptr.
   */
   TreeNode(DataType data) :
      m_data(std::move(data))
   {
   }

   /**
   * @brief TreeNode performs a copy-construction of the specified TreeNode.
   *
   * The nodes in the TreeNode are deep-copied, while the data contained in the tree is
   * shallow-copied.
   */
   TreeNode(const TreeNode<DataType>& other) :
      m_data(other.m_data)
   {
      Copy(other, *this);
   }

   /**
   * @brief Destroys the TreeNode and all TreeNodes under it.
   */
   ~TreeNode()
   {
      DetachFromTree();

      if (m_childCount == 0)
      {
         m_parent = nullptr;
         m_firstChild = nullptr;
         m_lastChild = nullptr;
         m_previousSibling = nullptr;
         m_nextSibling = nullptr;

         return;
      }

      assert(m_firstChild && m_lastChild);

      TreeNode<DataType>* childNode = m_firstChild;
      TreeNode<DataType>* nextNode = nullptr;

      while (childNode != nullptr)
      {
         nextNode = childNode->m_nextSibling;
         delete childNode;
         childNode = nextNode;
      }

      m_parent = nullptr;
      m_firstChild = nullptr;
      m_lastChild = nullptr;
      m_previousSibling = nullptr;
      m_nextSibling = nullptr;
   }

   /**
   * @brief Assignment operator.
   */
   TreeNode<DataType>& operator=(TreeNode<DataType> other) noexcept
   {
      swap(*this, other);
      return *this;
   }

   /**
   * @brief Swaps all member variables of the left-hand side with that of the right-hand side.
   */
   friend void swap(TreeNode<DataType>& lhs, TreeNode<DataType>& rhs)
      noexcept(noexcept(swap(lhs.m_data, rhs.m_data)))
   {
      // Enable Argument Dependent Lookup (ADL):
      using std::swap;

      swap(lhs.m_parent, rhs.m_parent);
      swap(lhs.m_firstChild, rhs.m_firstChild);
      swap(lhs.m_lastChild, rhs.m_lastChild);
      swap(lhs.m_previousSibling, rhs.m_previousSibling);
      swap(lhs.m_nextSibling, rhs.m_nextSibling);
      swap(lhs.m_data, rhs.m_data);
      swap(lhs.m_childCount, rhs.m_childCount);
      swap(lhs.m_visited, rhs.m_visited);
   }

   /**
   * @brief Detaches and then deletes the TreeNode from the Tree it's part of.
   */
   void DeleteFromTree() noexcept
   {
      delete this;
   }

   /**
   * @returns The encapsulated data.
   */
   DataType* operator->() noexcept
   {
      return &m_data;
   }

   /**
   * @overload
   */
   const DataType* operator->() const noexcept
   {
      return &m_data;
   }

   /**
   * @brief MarkVisited sets node visitation status.
   *
   * @param[in] visited             Whether the node should be marked as having been visited.
   */
   void MarkVisited(const bool visited = true) noexcept
   {
      m_visited = visited;
   }

   /**
   * @returns True if the node has been marked as visited.
   */
   constexpr bool HasBeenVisited() const noexcept
   {
      return m_visited;
   }

   /**
   * @brief PrependChild will prepend the specified TreeNode as the first child of the TreeNode.
   *
   * @param[in] child               The new TreeNode to set as the first child of the TreeNode.
   *
   * @returns A std::shared_ptr to the newly appended child.
   */
   TreeNode<DataType>* PrependChild(TreeNode<DataType>& child) noexcept
   {
      child.m_parent = this;

      if (!m_firstChild)
      {
         return AddFirstChild(child);
      }

      assert(m_firstChild);

      m_firstChild->m_previousSibling = &child;
      m_firstChild->m_previousSibling->m_nextSibling = m_firstChild;
      m_firstChild = m_firstChild->m_previousSibling;

      m_childCount++;

      return m_firstChild;
   }

   /**
   * @brief PrependChild will construct and prepend a new TreeNode as the first child of the
   * TreeNode.
   *
   * @param[in] data                The underlying data to be stored in the new TreeNode.
   *
   * @returns The newly prepended TreeNode.
   */
   TreeNode<DataType>* PrependChild(const DataType& data)
   {
      const auto* newNode = new TreeNode<DataType>(data);
      return PrependChild(*newNode);
   }

   /**
   * @overload
   */
   TreeNode<DataType>* PrependChild(DataType&& data)
   {
      auto* const newNode = new TreeNode<DataType>(std::move(data));
      return PrependChild(*newNode);
   }

   /**
   * @brief AppendChild will append the specified TreeNode as a child of the TreeNode.
   *
   * @param[in] child               The new TreeNode to set as the last child of the TreeNode.
   *
   * @returns A std::shared_ptr to the newly appended child.
   */
   TreeNode<DataType>* AppendChild(TreeNode<DataType>& child) noexcept
   {
      child.m_parent = this;

      if (!m_lastChild)
      {
         return AddFirstChild(child);
      }

      assert(m_lastChild);

      m_lastChild->m_nextSibling = &child;
      m_lastChild->m_nextSibling->m_previousSibling = m_lastChild;
      m_lastChild = m_lastChild->m_nextSibling;

      m_childCount++;

      return m_lastChild;
   }

   /**
   * @brief AppendChild will construct and append a new TreeNode as the last child of the TreeNode.
   *
   * @param[in] data                The underlying data to be stored in the new TreeNode.
   *
   * @returns The newly appended TreeNode.
   */
   TreeNode<DataType>* AppendChild(const DataType& data)
   {
      auto* const newNode = new TreeNode<DataType>(data);
      return AppendChild(*newNode);
   }

   /**
   * @overload
   */
   TreeNode<DataType>* AppendChild(DataType&& data)
   {
      auto* const newNode = new TreeNode<DataType>(std::move(data));
      return AppendChild(*newNode);
   }

   /**
   * @returns The underlying data stored in the TreeNode.
   */
   DataType& GetData() noexcept
   {
      return m_data;
   }

   /**
   * @overload
   */
   const DataType& GetData() const noexcept
   {
      return m_data;
   }

   /**
   * @returns A std::shared_ptr to the TreeNode's parent, if it exists; nullptr otherwise.
   */
   constexpr TreeNode<DataType>* const GetParent() const noexcept
   {
      return m_parent;
   }

   /**
   * @returns A std::shared_ptr to the TreeNode's first child.
   */
   constexpr TreeNode<DataType>* const GetFirstChild() const noexcept
   {
      return m_firstChild;
   }

   /**
   * @returns A std::shared_ptr to the TreeNode's last child.
   */
   constexpr TreeNode<DataType>* const GetLastChild() const noexcept
   {
      return m_lastChild;
   }

   /**
   * @returns A std::shared_ptr to the TreeNode's next sibling.
   */
   constexpr TreeNode<DataType>* const GetNextSibling() const noexcept
   {
      return m_nextSibling;
   }

   /**
   * @returns A std::shared_ptr to the TreeNode's previous sibling.
   */
   constexpr TreeNode<DataType>* const GetPreviousSibling() const noexcept
   {
      return m_previousSibling;
   }

   /**
   * @returns True if this node has direct descendants.
   */
   constexpr bool HasChildren() const noexcept
   {
      return m_childCount > 0;
   }

   /**
   * @returns The number of direct descendants that this node has.
   *
   * @note This does not include grandchildren.
   */
   constexpr unsigned int GetChildCount() const noexcept
   {
      return m_childCount;
   }

   /**
   * @returns The total number of descendant nodes belonging to the node.
   */
   size_t CountAllDescendants() noexcept
   {
      const auto nodeCount = std::count_if(
         Tree<DataType>::PostOrderIterator(this),
         Tree<DataType>::PostOrderIterator(),
         [](Tree<DataType>::const_reference)
      {
         return true;
      });

      return nodeCount - 1;
   }

   /**
   * @brief SortChildren performs a merge sort of the direct descendants nodes.
   *
   * @param[in] comparator          The function to be used as the basis for the sorting comparison.
   */
   void SortChildren(
      const std::function<bool(const TreeNode<DataType>&, const TreeNode<DataType>&)>& comparator)
   {
      if (!m_firstChild)
      {
         return;
      }

      MergeSort(m_firstChild, comparator);
   }

private:

   /**
   * @brief MergeSort is the main entry point into the merge sort implementation.
   *
   * @param[in] list                The first TreeNode in the list to be sorted.
   * @param[in] comparator          The comparator function to be called to figure out which node
   *                                is the lesser of the two.
   */
   void MergeSort(
      TreeNode<DataType>*& list,
      const std::function<bool(const TreeNode<DataType>&, const TreeNode<DataType>&)>& comparator)
      noexcept(noexcept(comparator))
   {
      if (!list || !list->m_nextSibling)
      {
         return;
      }

      TreeNode<DataType>* head = list;
      TreeNode<DataType>* lhs = nullptr;
      TreeNode<DataType>* rhs = nullptr;

      DivideList(head, lhs, rhs);

      assert(lhs);
      assert(rhs);

      MergeSort(lhs, comparator);
      MergeSort(rhs, comparator);

      list = MergeSortedHalves(lhs, rhs, comparator);
   }

   /**
   * @brief DivideList is a helper function that will divide the specified TreeNode list in two.
   *
   * @param[in] head                The head of the TreeNode list to be divided in two.
   * @param[out] lhs                The first TreeNode of the left hand side list.
   * @param[out] rhs                The first TreeNode of the right hand side list.
   */
   void DivideList(
      TreeNode<DataType>* head,
      TreeNode<DataType>*& lhs,
      TreeNode<DataType>*& rhs) noexcept
   {
      if (!head || !head->m_nextSibling)
      {
         return;
      }

      TreeNode<DataType>* tortoise = head;
      TreeNode<DataType>* hare = head->m_nextSibling;

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

   /**
   * @brief MergeSortedHalves is a helper function that will merge the sorted halves.
   *
   * @param[in] lhs                 The first node of the sorted left half.
   * @param[in] rhs                 The first node of the sorted right half.
   *
   * @returns The first node of the merged TreeNode list.
   */
   TreeNode<DataType>* MergeSortedHalves(
      TreeNode<DataType>*& lhs,
      TreeNode<DataType>*& rhs,
      const std::function<bool(const TreeNode<DataType>&, const TreeNode<DataType>&)>& comparator)
      noexcept(noexcept(comparator))
   {
      TreeNode<DataType>* result = nullptr;
      if (comparator(*lhs, *rhs))
      {
         result = lhs;
         lhs = lhs->m_nextSibling;
      }
      else
      {
         result = rhs;
         rhs = rhs->m_nextSibling;
      }

      result->m_previousSibling = nullptr;

      TreeNode<DataType>* tail = result;

      while (lhs && rhs)
      {
         if (comparator(*lhs, *rhs))
         {
            tail->m_nextSibling = lhs;
            tail = tail->m_nextSibling;

            lhs = lhs->m_nextSibling;

            if (lhs)
            {
               lhs->m_previousSibling = nullptr;
            }
         }
         else
         {
            tail->m_nextSibling = rhs;
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
         tail->m_nextSibling = lhs;
         tail = tail->m_nextSibling;

         lhs = lhs->m_nextSibling;

         if (lhs)
         {
            lhs->m_previousSibling = nullptr;
         }
      }

      while (rhs)
      {
         tail->m_nextSibling = rhs;
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
   * @brief AddFirstChild is a helper function to make it easier to add the first descendant.
   *
   * @param[in] child               The TreeNode to be added as a child.
   *
   * @returns The newly added node.
   */
   TreeNode<DataType>* AddFirstChild(TreeNode<DataType>& child) noexcept
   {
      assert(m_childCount == 0);

      m_firstChild = &child;
      m_lastChild = m_firstChild;

      m_childCount++;

      return m_firstChild;
   }

   /**
   * @brief Helper function to recursively copy the specified |source| TreeNode and all its
   * descendants.
   *
   * @param[in] source              The TreeNode to copy information from.
   * @param[out] sink               The TreeNode to place a copy of the information into.
   */
   void Copy(const TreeNode<DataType>& source, TreeNode<DataType>& sink)
   {
      if (!source.HasChildren())
      {
         return;
      }

      std::for_each(
         Tree<DataType>::SiblingIterator(source.GetFirstChild()),
         Tree<DataType>::SiblingIterator(),
         [&](Tree<DataType>::const_reference node)
      {
         sink.AppendChild(node.GetData());
      });

      auto sourceItr = Tree<DataType>::SiblingIterator{ source.GetFirstChild() };
      auto sinkItr = Tree<DataType>::SiblingIterator{ sink.GetFirstChild() };

      const auto end = Tree<DataType>::SiblingIterator{ };
      while (sourceItr != end)
      {
         Copy(*sourceItr++, *sinkItr++);
      }
   }

   /**
   * @brief Removes the TreeNode from the tree structure, updating all surrounding links
   * as appropriate.
   *
   * @note This function does not actually delete the node.
   *
   * @returns A pointer to the detached TreeNode. This returned TreeNode can safely be deleted
   * once detached.
   */
   TreeNode<DataType>* DetachFromTree() noexcept
   {
      if (m_previousSibling && m_nextSibling)
      {
         m_previousSibling->m_nextSibling = m_nextSibling;
         m_nextSibling->m_previousSibling = m_previousSibling;
      }
      else if (m_previousSibling)
      {
         m_previousSibling->m_nextSibling = nullptr;
      }
      else if (m_nextSibling)
      {
         m_nextSibling->m_previousSibling = nullptr;
      }

      if (!m_parent)
      {
         return this;
      }

      if (m_parent->m_firstChild == m_parent->m_lastChild)
      {
         m_parent->m_firstChild = nullptr;
         m_parent->m_lastChild = nullptr;
      }
      else if (m_parent->m_firstChild == this)
      {
         assert(m_parent->m_firstChild->m_nextSibling);
         m_parent->m_firstChild = m_parent->m_firstChild->m_nextSibling;
      }
      else if (m_parent->m_lastChild == this)
      {
         assert(m_parent->m_lastChild->m_previousSibling);
         m_parent->m_lastChild = m_parent->m_lastChild->m_previousSibling;
      }

      m_parent->m_childCount--;

      return this;
   }

   TreeNode<DataType>* m_parent{ nullptr };
   TreeNode<DataType>* m_firstChild{ nullptr };
   TreeNode<DataType>* m_lastChild{ nullptr };
   TreeNode<DataType>* m_previousSibling{ nullptr };
   TreeNode<DataType>* m_nextSibling{ nullptr };

   DataType m_data{ };

   unsigned int m_childCount{ 0 };

   bool m_visited{ false };
};

/**
* The Tree class declares a basic n-ary tree, built on top of templatized TreeNode nodes.
*
* Each tree consists of a simple head TreeNode and nothing else.
*/
template<typename DataType>
class Tree
{
public:
   friend class TreeNode<DataType>;

   class Iterator;
   class PreOrderIterator;
   class PostOrderIterator;
   class LeafIterator;
   class SiblingIterator;

   // Typedefs needed for STL compliance:
   typedef TreeNode<DataType>                value_type;
   typedef TreeNode<DataType>&               reference;
   typedef const TreeNode<DataType>&         const_reference;

   /**
   * @brief Default constructor.
   */
   Tree() :
      m_head(new TreeNode<DataType>{ })
   {
   }

   /**
   * @brief Tree constructs a new Tree with the provided data encapsulated in a new
   * TreeNode.
   */
   Tree(DataType data) :
      m_head(new TreeNode<DataType>{ data })
   {
   }

   /**
   * @brief Copy constructor.
   */
   Tree(const Tree<DataType>& other) :
      m_head(new TreeNode<DataType>{ *other.m_head })
   {
   }

   /**
   * @brief Assignment operator.
   */
   Tree<DataType>& operator=(Tree<DataType> other) noexcept(noexcept(swap(*this, other)))
   {
      swap(*this, other);
      return *this;
   }

   /**
   * @brief Swaps all member variables of the left-hand side with that of the right-hand side.
   */
   friend void swap(Tree<DataType>& lhs, Tree<DataType>& rhs)
      noexcept(noexcept(swap(lhs.m_head, rhs.m_head)))
   {
      // Enable Argument Dependent Lookup (ADL):
      using std::swap;

      swap(lhs.m_head, rhs.m_head);
   }

   /**
   * @brief Deletes the head TreeNode, which, in turn, will trigger a deletion of every
   * TreeNode in the Tree.
   */
   ~Tree()
   {
      delete m_head;
   }

   /**
   * @returns A std::shared_ptr to the head TreeNode.
   */
   TreeNode<DataType>* GetHead() const noexcept
   {
      return m_head;
   }

   /**
   * @brief Computes the size of nodes in the Tree.
   *
   * @complexity Linear in the size of the sub-tree.
   *
   * @returns The total number of nodes in the Tree (both leaf and non-leaf).
   */
   size_t Size() const noexcept
   {
      return std::count_if(std::begin(*this), std::end(*this),
         [] (const auto&)
      {
         return true;
      });
   }

   /**
   * @returns The zero-indexed depth of the TreeNode in its Tree.
   */
   static unsigned int Depth(TreeNode<DataType> node) noexcept
   {
      unsigned int depth = 0;

      TreeNode<DataType>* nodePtr = &node;
      while (nodePtr->GetParent())
      {
         ++depth;
         nodePtr = nodePtr->GetParent();
      }

      return depth;
   }

   /**
   * @returns A pre-order iterator that will iterate over all TreeNodes in the tree.
   */
   typename Tree::PreOrderIterator beginPreOrder() const noexcept
   {
      const auto iterator = Tree<DataType>::PreOrderIterator{ m_head };
      return iterator;
   }

   /**
   * @returns A pre-order iterator pointing "past" the end of the tree.
   */
   typename Tree::PreOrderIterator endPreOrder() const noexcept
   {
      const auto iterator = Tree<DataType>::PreOrderIterator{ nullptr };
      return iterator;
   }

   /**
   * @returns A post-order iterator that will iterator over all nodes in the tree, starting
   * with the head of the Tree.
   */
   typename Tree::PostOrderIterator begin() const noexcept
   {
      const auto iterator = Tree<DataType>::PostOrderIterator{ m_head };
      return iterator;
   }

   /**
   * @returns A post-order iterator that points past the end of the Tree.
   */
   typename Tree::PostOrderIterator end() const noexcept
   {
      const auto iterator = Tree<DataType>::PostOrderIterator{ nullptr };
      return iterator;
   }

   /**
   * @returns An iterator that will iterator over all leaf nodes in the Tree, starting with the
   * left-most leaf in the Tree.
   */
   typename Tree::LeafIterator beginLeaf() const noexcept
   {
      const auto iterator = Tree<DataType>::LeafIterator{ m_head };
      return iterator;
   }

   /**
   * @return A LeafIterator that points past the last leaf TreeNode in the Tree.
   */
   typename Tree::LeafIterator endLeaf() const noexcept
   {
      const auto iterator = Tree<DataType>::LeafIterator{ nullptr };
      return iterator;
   }

private:
   TreeNode<DataType>* m_head{ nullptr };
};

/**
* @brief The Iterator class
*
* This is the base iterator class that all other iterators (sibling, leaf, post-, pre-, and
* in-order) will derive from. This class can only instantiated by derived types.
*/
template<typename DataType>
class Tree<DataType>::Iterator
{
public:
   // Typedefs needed for STL compliance:
   typedef DataType                             value_type;
   typedef DataType*                            pointer;
   typedef DataType&                            reference;
   typedef const DataType&                      const_reference;
   typedef std::size_t                          size_type;
   typedef std::ptrdiff_t                       difference_type;
   typedef std::forward_iterator_tag            iterator_category;

   /**
   * @returns True if the Tree::Iterator points to a valid TreeNode; false otherwise.
   */
   explicit operator bool() const noexcept
   {
      const auto isValid = (m_currentNode != nullptr);
      return isValid;
   }

   /**
   * @returns The TreeNode pointed to by the Tree::Iterator.
   */
   TreeNode<DataType>& operator*() noexcept
   {
      return *m_currentNode;
   }

   /**
   * @overload
   */
   const TreeNode<DataType>& operator*() const noexcept
   {
      return *m_currentNode;
   }

   /**
   * @returns A pointer to the TreeNode.
   */
   TreeNode<DataType>* const operator&() noexcept
   {
      return m_currentNode;
   }

   /**
   * @overload
   */
   const TreeNode<DataType>* const operator&() const noexcept
   {
      return m_currentNode;
   }

   /**
   * @returns A pointer to the TreeNode pointed to by the Tree:Iterator.
   */
   TreeNode<DataType>* const operator->() noexcept
   {
      return m_currentNode;
   }

   /**
   * @overload
   */
   const TreeNode<DataType>* const operator->() const noexcept
   {
      return m_currentNode;
   }

   /**
   * @returns True if the Iterator points to the same node as the other Iterator,
   * and false otherwise.
   */
   bool operator==(const Iterator& other) const noexcept
   {
      return m_currentNode == other.m_currentNode;
   }

   /**
   * @returns True if the Iterator points to the same node as the other Iterator,
   * and false otherwise.
   */
   bool operator!=(const Iterator& other) const noexcept
   {
      return m_currentNode != other.m_currentNode;
   }

protected:
   /**
   * Default constructor.
   */
   Iterator() noexcept = default;

   /**
   * Copy constructor.
   */
   explicit Iterator(const Iterator& other) noexcept :
      m_currentNode(other.m_currentNode),
      m_startingNode(other.m_startingNode),
      m_endingNode(other.m_endingNode)
   {
   }

   /**
   * Constructs a Iterator started at the specified node.
   */
   explicit Iterator(const TreeNode<DataType>* node) noexcept :
      m_currentNode(const_cast<TreeNode<DataType>*>(node)),
      m_startingNode(const_cast<TreeNode<DataType>*>(node))
   {
   }

   TreeNode<DataType>* m_currentNode{ nullptr };
   TreeNode<DataType>* m_startingNode{ nullptr };
   TreeNode<DataType>* m_endingNode{ nullptr };
};

/**
* @brief The PreOrderIterator class
*/
template<typename DataType>
class Tree<DataType>::PreOrderIterator : public Tree<DataType>::Iterator
{
public:
   /**
   * Default constructor.
   */
   PreOrderIterator() noexcept = default;

   /**
   * Constructs an iterator that starts and ends at the specified node.
   */
   explicit PreOrderIterator(const TreeNode<DataType>* node) noexcept :
      Iterator(node)
   {
      if (!node)
      {
         return;
      }

      m_endingNode = node->GetNextSibling();
   }

   /**
   * Prefix increment operator.
   */
   typename Tree::PreOrderIterator& operator++() noexcept
   {
      assert(m_currentNode);
      auto* traversingNode = m_currentNode;

      if (traversingNode->HasChildren())
      {
         traversingNode = traversingNode->GetFirstChild();
      }
      else if (traversingNode->GetNextSibling())
      {
         traversingNode = traversingNode->GetNextSibling();
      }
      else
      {
         while (traversingNode->GetParent() && !traversingNode->GetParent()->GetNextSibling())
         {
            traversingNode = traversingNode->GetParent();
         }

         if (traversingNode->GetParent())
         {
            traversingNode = traversingNode->GetParent()->GetNextSibling();
         }
         else
         {
            traversingNode = nullptr;
         }
      }

      m_currentNode = (traversingNode != m_endingNode) ? traversingNode : nullptr;
      return *this;
   }

   /**
   * Postfix increment operator.
   */
   typename Tree::PreOrderIterator operator++(int) noexcept
   {
      const auto result = *this;
      ++(*this);

      return result;
   }
};

/**
* @brief The PostOrderIterator class
*/
template<typename DataType>
class Tree<DataType>::PostOrderIterator : public Tree<DataType>::Iterator
{
public:
   /**
   * Default constructor.
   */
   PostOrderIterator() noexcept = default;

   /**
   * Constructs an iterator that starts and ends at the specified node.
   */
   explicit PostOrderIterator(const TreeNode<DataType>* node) noexcept :
      Iterator(node)
   {
      if (!node)
      {
         return;
      }

      if (node->GetNextSibling())
      {
         auto* traversingNode = node->GetNextSibling();
         while (traversingNode->HasChildren())
         {
            traversingNode = traversingNode->GetFirstChild();
         }

         m_endingNode = traversingNode;
      }

      while (node->GetFirstChild())
      {
         node = node->GetFirstChild();
      }

      assert(node);
      m_currentNode = const_cast<TreeNode<DataType>*>(node);
   }

   /**
   * Prefix increment operator.
   */
   typename Tree::PostOrderIterator& operator++() noexcept
   {
      assert(m_currentNode);
      auto* traversingNode = m_currentNode;

      if (traversingNode->HasChildren() && !m_traversingUpTheTree)
      {
         while (traversingNode->GetFirstChild())
         {
            traversingNode = traversingNode->GetFirstChild();
         }
      }
      else if (traversingNode->GetNextSibling())
      {
         m_traversingUpTheTree = false;

         traversingNode = traversingNode->GetNextSibling();
         while (traversingNode->HasChildren())
         {
            traversingNode = traversingNode->GetFirstChild();
         }
      }
      else
      {
         m_traversingUpTheTree = true;

         traversingNode = traversingNode->GetParent();
      }

      m_currentNode = (traversingNode != m_endingNode) ? traversingNode : nullptr;
      return *this;
   }

   /**
   * Postfix increment operator.
   */
   typename Tree::PostOrderIterator operator++(int) noexcept
   {
      const auto result = *this;
      ++(*this);

      return result;
   }

private:
   bool m_traversingUpTheTree{ false };
};

/**
* @brief The LeafIterator class
*/
template<typename DataType>
class Tree<DataType>::LeafIterator : public Tree<DataType>::Iterator
{
public:
   /**
   * Default constructor.
   */
   LeafIterator() noexcept = default;

   /**
   * Constructs an iterator that starts at the specified node and iterates to the end.
   */
   explicit LeafIterator(const TreeNode<DataType>* node) noexcept :
      Iterator(node)
   {
      if (!node)
      {
         return;
      }

      if (node->HasChildren())
      {
         auto* firstNode = node;
         while (firstNode->GetFirstChild())
         {
            firstNode = firstNode->GetFirstChild();
         }

         m_currentNode = const_cast<TreeNode<DataType>*>(firstNode);
      }

      if (node->GetNextSibling())
      {
         auto* lastNode = node->GetNextSibling();
         while (lastNode->HasChildren())
         {
            lastNode = lastNode->GetFirstChild();
         }

         m_endingNode = lastNode;
      }
   }

   /**
   * Prefix increment operator.
   */
   typename Tree::LeafIterator& operator++() noexcept
   {
      assert(m_currentNode);
      auto* traversingNode = m_currentNode;

      if (traversingNode->HasChildren())
      {
         while (traversingNode->GetFirstChild())
         {
            traversingNode = traversingNode->GetFirstChild();
         }
      }
      else if (traversingNode->GetNextSibling())
      {
         traversingNode = traversingNode->GetNextSibling();

         while (traversingNode->GetFirstChild())
         {
            traversingNode = traversingNode->GetFirstChild();
         }
      }
      else if (traversingNode->GetParent())
      {
         while (traversingNode->GetParent() && !traversingNode->GetParent()->GetNextSibling())
         {
            traversingNode = traversingNode->GetParent();
         }

         if (traversingNode->GetParent())
         {
            traversingNode = traversingNode->GetParent()->GetNextSibling();

            while (traversingNode && traversingNode->HasChildren())
            {
               traversingNode = traversingNode->GetFirstChild();
            }
         }
         else
         {
            traversingNode = nullptr;
         }
      }

      m_currentNode = (traversingNode != m_endingNode) ? traversingNode : nullptr;
      return *this;
   }

   /**
   * Postfix increment operator.
   */
   typename Tree::LeafIterator operator++(int) noexcept
   {
      const auto result = *this;
      ++(*this);

      return result;
   }
};

/**
* @brief The SiblingIterator class
*/
template<typename DataType>
class Tree<DataType>::SiblingIterator : public Tree<DataType>::Iterator
{
public:
   /**
   * Default constructor.
   */
   SiblingIterator() noexcept = default;

   /**
   * Constructs an iterator that starts at the specified node and iterates to the end.
   */
   explicit SiblingIterator(const TreeNode<DataType>* node) noexcept :
      Iterator(node)
   {
   }

   /**
   * Prefix increment operator.
   */
   typename Tree::SiblingIterator& operator++() noexcept
   {
      if (m_currentNode)
      {
         m_currentNode = m_currentNode->GetNextSibling();
      }

      return *this;
   }

   /**
   * Postfix increment operator.
   */
   typename Tree::SiblingIterator operator++(int) noexcept
   {
      const auto result = *this;
      ++(*this);

      return result;
   }

private:
   TreeNode<DataType>* m_parent{ nullptr };
};