#include "treeNode.h"

TreeNode::TreeNode()
   : m_parent(nullptr),
     m_firstChild(nullptr),
     m_lastChild(nullptr),
     m_previousSibling(nullptr),
     m_nextSibling(nullptr)
{
}

TreeNode::TreeNode(const T& otherNode)
   : m_parent(nullptr),
     m_firstChild(nullptr),
     m_lastChild(nullptr),
     m_previousSibling(nullptr),
     m_nextSibling(nullptr),
     m_data(otherNode)
{
}

TreeNode::~TreeNode()
{
}

