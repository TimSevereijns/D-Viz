#include "tree.h"

#include "treeNode.h"

template<class T>
Tree::Tree()
   : m_head(TreeNode<T>())
{
}

template<class T>
Tree::~Tree()
{
}

