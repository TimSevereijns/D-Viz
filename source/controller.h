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

class Controller
{
   friend class GLCanvas;

   public:
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
      void SearchTreeMap(
         bool shouldSearchFiles,
         bool shouldSearchDirectories);

      /**
       * @brief HighlightMatchingExtension
       *
       * @param selectedNode
       */
      void HighlightAllMatchingExtension(const TreeNode<VizNode>& targetNode);

      /**
       * @brief HighlightDescendants
       *
       * @param selectedNode
       */
      void HighlightDescendants(const TreeNode<VizNode>& node);

      /**
       * @brief HighlightAncestors
       *
       * @param selectedNode
       */
      void HighlightAncestors(const TreeNode<VizNode>& node);

      /**
       * @brief ClearHighlightedNodes
       */
      void ClearHighlightedNodes();

      /**
       * @brief HighlightNodes
       */
      void PaintHighlightedNodes();

      /**
       * @brief SelectNode
       * @param selectedNode
       */
      void SelectNode(const TreeNode<VizNode>* const node);

      /**
       * @brief SelectNodeViaRay
       *
       * @param camera
       * @param ray
       */
      void SelectNodeViaRay(
         const Camera& camera,
         const Qt3DCore::QRay3D& ray);

      /**
       * @brief Helper function to set the specifed vertex and block count in the bottom status bar.
       *
       * @param[in] vertexCount        The readout value.
       */
      void PrintMetadataToStatusBar(const std::uint32_t nodeCount);

      /**
       * @brief Converts the given size of the file from bytes to the most human readable units.
       *
       * @param[in] sizeInBytes        The size (in bytes) to be converted to a more appropriate unit.
       *
       * @returns A std::pair encapsulating the converted file size, and corresponding unit readout
       * string.
       */
      static std::pair<double, std::wstring> ConvertFileSizeToMostAppropriateUnits(
         double sizeInBytes);

      /**
       * @brief Computes the absolute file path of the selected node by traveling up tree.
       *
       * @param[in] node               The selected node.
       *
       * @returns The absolute file path.
       */
      static std::wstring ResolveCompleteFilePath(const TreeNode<VizNode>& node);

      /**
       * @brief Opens the selected file in Windows File Explorer.
       *
       * @param[in] selectedNode       The node that represents the file to open.
       */
      static void ShowInFileExplorer(const TreeNode<VizNode>& node);

      /**
       * @brief PrintSelectionDetailsToStatusBar
       */
      void PrintSelectionDetailsToStatusBar();

   private:

      MainWindow* m_mainWindow{ nullptr };

      const TreeNode<VizNode>* m_selectedNode{ nullptr };

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
