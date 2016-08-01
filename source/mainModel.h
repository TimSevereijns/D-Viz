#ifndef MAINMODEL_H
#define MAINMODEL_H

#include "DataStructs/light.h"
#include "DataStructs/vizNode.h"
#include "DriveScanner/driveScanner.h"
#include "ThirdParty/Tree.hpp"

#include <memory>
#include <vector>

#include <QVector3D>

class MainWindow;
class GLCanvas;

class MainModel
{
   friend class GLCanvas;

   public:
      MainModel();

      /**
       * @returns True if the visualization is not null;
       */
      bool HasVisualizationBeenLoaded() const;

      /**
       * @brief GenerateNewVisualization
       *
       * @param parameters
       */
      void GenerateNewVisualization(VisualizationParameters& parameters);

      /**
       * @brief GetSelectedNode
       * @return
       */
      const TreeNode<VizNode>* const GetSelectedNode() const;

      /**
       * @brief GetTree
       * @return
       */
      Tree<VizNode>& GetTree();

      /**
       * @brief GetTree
       * @return
       */
      const Tree<VizNode>& GetTree() const;

      /**
       * @brief GetVisualizationParameters
       * @return
       */
      const VisualizationParameters& GetVisualizationParameters() const;

      /**
       * @brief SetVisualizationParameters
       */
      void SetVisualizationParameters(const VisualizationParameters& parameters);

      /**
       * @brief GetSelectedNodes
       * @return
       */
      const std::vector<const TreeNode<VizNode>*> GetHighlightedNodes() const;

      /**
       * @brief SetView
       * @param window
       */
      void SetView(MainWindow* window);

      /**
       * @brief ParseResults
       * @param results
       */
      void ParseResults(const std::shared_ptr<Tree<VizNode>>& results);

      /**
       * @brief UpdateBoundingBoxes
       */
      void UpdateBoundingBoxes();

      /**
       * @brief SearchTreeMap
       *
       * @param searchFiles
       * @param searchDirectories
       *
       * @return
       */
      std::vector<const TreeNode<VizNode>*> SearchTreeMap(
         bool shouldSearchFiles,
         bool shouldSearchDirectories);

      /**
       * @brief HighlightMatchingExtension
       *
       * @param selectedNode
       */
      void HighlightAllMatchingExtension(const TreeNode<VizNode>& selectedNode);

      /**
       * @brief HighlightDescendants
       *
       * @param selectedNode
       */
      void HighlightDescendants(const TreeNode<VizNode>& selectedNode);

      /**
       * @brief HighlightAncestors
       *
       * @param selectedNode
       */
      void HighlightAncestors(const TreeNode<VizNode>& selectedNode);

      /**
       * @brief ClearHighlightedNodes
       */
      void ClearHighlightedNodes();

      /**
       * @brief HighlightNodes
       */
      void PaintHighlightNodes();

   private:

      MainWindow* m_mainWindow{ nullptr };

      TreeNode<VizNode>* m_selectedNode{ nullptr };

      std::unique_ptr<VisualizationModel> m_treeMap{ nullptr };

      std::vector<const TreeNode<VizNode>*> m_highlightedNodes;

      std::vector<Light> m_lights
      {
         Light{ },
         Light{ QVector3D{ 0.0f, 80.0f, 0.0f } },
         Light{ QVector3D{ 0.0f, 80.0f, -VisualizationModel::ROOT_BLOCK_DEPTH } },
         Light{ QVector3D{ VisualizationModel::ROOT_BLOCK_WIDTH, 80.0f, 0.0f } },
         Light{ QVector3D{ VisualizationModel::ROOT_BLOCK_WIDTH, 80.0f,
            -VisualizationModel::ROOT_BLOCK_DEPTH } }
      };

      VisualizationParameters m_visualizationParameters;
};

#endif // MAINMODEL_H
