#ifndef VIEWCALLBACKS_H
#define VIEWCALLBACKS_H

#include <functional>
#include <vector>

#include "../ThirdParty/Tree.hpp"
#include "DataStructs/vizNode.h"

using VectorOfConstNodes = std::vector<const TreeNode<VizNode>*>;

struct ViewCallbacks
{
   ViewCallbacks(
      const std::function<void (VectorOfConstNodes&)>& renderer,
      const std::function<void (VectorOfConstNodes&)>& highlightClearer,
      const std::function<void ()>& selectionClearer)
      :
      RenderNodes{ renderer },
      ClearHighlightedNodes{ highlightClearer },
      ClearSelectedNode{ selectionClearer }
   {
   }

   const std::function<void (VectorOfConstNodes&)> RenderNodes;
   const std::function<void (VectorOfConstNodes&)> ClearHighlightedNodes;
   const std::function<void ()> ClearSelectedNode;
};

#endif // VIEWCALLBACKS_H
