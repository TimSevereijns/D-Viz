#ifndef TREENODE_H
#define TREENODE_H

template<T>
class TreeNode
{
   public:
      TreeNode();
      TreeNode(const T& otherNode);
      ~TreeNode();

      TreeNode<T>* m_parent;
      TreeNode<T>* m_firstChild;
      TreeNode<T>* m_lastChild;
      TreeNode<T>* m_previousSibling;
      TreeNode<T>* m_nextSibling;

      T m_data;
};

#endif // TREENODE_H
