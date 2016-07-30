#ifndef MAINMODEL_H
#define MAINMODEL_H

#include "DataStructs/light.h"
#include "DataStructs/vizNode.h"
#include "DriveScanner/driveScanner.h"
#include "ThirdParty/Tree.hpp"

#include <memory>
#include <vector>

#include <QVector3D>

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
       * @brief GetSelectedNodes
       * @return
       */
      const std::vector<const TreeNode<VizNode>*> GetHighlightedNodes() const;

   private:

      void ScanDrive(VisualizationParameters& vizParameters);

      TreeNode<VizNode>* m_selectedNode{ nullptr };

      std::vector<const TreeNode<VizNode>*> m_highlightedNodes;

      std::unique_ptr<VisualizationModel> m_theVisualization{ nullptr };

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

      DriveScanner m_scanner;
};

#endif // MAINMODEL_H
