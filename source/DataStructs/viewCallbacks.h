#ifndef VIEWCALLBACKS_H
#define VIEWCALLBACKS_H

#include <functional>
#include <vector>

#include "../ThirdParty/Tree.hpp"
#include "DataStructs/vizNode.h"

//template<
//   typename RenderCallbackType,
//   typename HighlightClearingCallbackType,
//   typename SelectionClearingCallbackType>
//struct CallbackParameters
//{
//   CallbackParameters(
//      const RenderCallbackType& renderCallback,
//      const HighlightClearingCallbackType& highlightCallback,
//      const SelectionClearingCallbackType& selectionCallback)
//      :
//      PaintNodes{ renderCallback },
//      ClearHighlightedNodes{ highlightCallback },
//      ClearSelectedNodes{ selectionCallback }
//   {
//   }

//   const RenderCallbackType& PaintNodes;
//   const HighlightClearingCallbackType& ClearHighlightedNodes;
//   const SelectionClearingCallbackType& ClearSelectedNodes;
//};

//template<
//   typename RenderCallbackType,
//   typename HighlightClearingCallbackType,
//   typename SelectionClearingCallbackType>
//auto MakeCallbackParams(
//   const RenderCallbackType& renderCallback,
//   const HighlightClearingCallbackType& highlightCallback,
//   const SelectionClearingCallbackType& selectionCallback)
//{
//   const CallbackParameters<
//      RenderCallbackType,
//      HighlightClearingCallbackType,
//      SelectionClearingCallbackType> callbacks
//   (
//      renderCallback,
//      highlightCallback,
//      selectionCallback
//   );

//   return callbacks;
//}

struct ViewCallbacks
{
   ViewCallbacks(
      const std::function<void (std::vector<const TreeNode<VizNode>*>&)>& renderer,
      const std::function<void (std::vector<const TreeNode<VizNode>*>&)>& highlightClearer,
      const std::function<void ()>& selectionClearer)
      :
      RenderNodes{ renderer },
      ClearHighlightedNodes{ highlightClearer },
      ClearSelectedNode{ selectionClearer }
   {
   }

   const std::function<void (std::vector<const TreeNode<VizNode>*>&)> RenderNodes;
   const std::function<void (std::vector<const TreeNode<VizNode>*>&)> ClearHighlightedNodes;
   const std::function<void ()> ClearSelectedNode;
};

#endif // VIEWCALLBACKS_H
