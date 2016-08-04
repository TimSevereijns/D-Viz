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
   public:
      /**
       * @brief Sets the MainWindow as the "view" for the controller.
       *
       * @param[in] window          The MainWindow to be used as the view.
       */
      void SetView(MainWindow* window);

      /**
       * @returns True if the visualization is not null;
       */
      bool HasVisualizationBeenLoaded() const;

      /**
       * @brief Generates a new visualization.
       *
       * @param[in] parameters
       */
      void GenerateNewVisualization();

      /**
       * @returns A pointer to the selected node.
       */
      const TreeNode<VizNode>* const GetSelectedNode() const;

      /**
       * @returns A reference to the tree that represents the most recent drive scan.
       */
      Tree<VizNode>& GetTree();

      /**
       * @overload
       */
      const Tree<VizNode>& GetTree() const;

      /**
       * @returns A reference to the current visualization parameters.
       */
      const VisualizationParameters& GetVisualizationParameters() const;

      /**
       * @brief Updates the visualization parameters.
       */
      void SetVisualizationParameters(const VisualizationParameters& parameters);

      /**
       * @returns A reference to the currently highlighted nodes. Highlighted nodes are distinct
       * from the selected node (of which there can be only one).
       */
      const std::vector<const TreeNode<VizNode>*>& GetHighlightedNodes() const;

      /**
       * @brief Parses the drive scan results.
       *
       * @param[in] results         The scan results to be parsed.
       */
      void ParseResults(const std::shared_ptr<Tree<VizNode>>& results);

      /**
       * @brief Updates the bounding boxes for the current treemap.
       */
      void UpdateBoundingBoxes();

      /**
       * @brief Searches the treemap for the search query contained within the search box.
       *
       * @todo Make this more generic, and pass in the search query.
       *
       * @param[in] searchQuery        String to search against.
       * @param[in] searchFiles        Pass in true to search files.
       * @param[in] searchDirectories  Pass in true to search directories.
       */
      void SearchTreeMap(
         const std::wstring& searchQuery,
         bool shouldSearchFiles,
         bool shouldSearchDirectories);

      /**
       * @brief Highlights all nodes in the tree whose extension matches that of the passed in node.
       *
       * @param[in] targetNode      The node whose extension is to be highlighted.
       */
      void HighlightAllMatchingExtension(const TreeNode<VizNode>& targetNode);

      /**
       * @brief Highlights all nodes that descendant from the passed in node.
       *
       * @param[in] node            The node whose descendants to highlight.
       */
      void HighlightDescendants(const TreeNode<VizNode>& node);

      /**
       * @brief Highlights all nodes that are ancestors of the passed in node.
       *
       * @param[in] node            The node whose ancestors are to be highlighted.
       */
      void HighlightAncestors(const TreeNode<VizNode>& node);

      /**
       * @brief Clears all highlighted nodes, as well as the selected node.
       */
      void ClearHighlightedNodes();

      /**
       * @brief Updates the visual representation of the highlighted nodes.
       */
      void PaintSelectedAndHighlightedNodes();

      /**
       * @brief Selects the passed in node.
       *
       * @param[in] node            A pointer to the node to be selected.
       */
      void SelectNode(const TreeNode<VizNode>* const node);

      /**
       * @brief Uses the passed in ray to select the nearest node from the perspective of the
       * camera.
       *
       * @todo Remove the camera object, and pass in the camera's position instead.
       *
       * @param[in] camera          The camera from which the ray was shot.
       * @param[in] ray             The picking ray.
       */
      void SelectNodeViaRay(
         const Camera& camera,
         const Qt3DCore::QRay3D& ray);

      /**
       * @brief Helper function to set the specifed vertex and block count in the bottom status bar.
       *
       * @param[in] nodeCount       The number of nodes that are currently selected or highlighted.
       */
      void PrintMetadataToStatusBar(const std::uint32_t nodeCount);

      /**
       * @brief Converts the given size of the file from bytes to the most human readable units.
       *
       * @param[in] sizeInBytes     The size (in bytes) to be converted to a more appropriate
       * unit.
       *
       * @returns A std::pair encapsulating the converted file size, and corresponding unit readout
       * string.
       */
      static std::pair<double, std::wstring> ConvertFileSizeToAppropriateUnits(
         double sizeInBytes);

      /**
       * @brief Computes the absolute file path of the selected node by traveling up tree.
       *
       * @param[in]                 The selected node.
       *
       * @returns The absolute file path.
       */
      static std::wstring ResolveCompleteFilePath(const TreeNode<VizNode>& node);

      /**
       * @brief Opens the selected file in Windows File Explorer.
       *
       * @param[in] node            The node that represents the file to open.
       */
      static void ShowInFileExplorer(const TreeNode<VizNode>& node);

      /**
       * @brief Prints selection details to the main window's status bar.
       */
      void PrintSelectionDetailsToStatusBar();

   private:

      template<typename LambdaType>
      void Highlight(LambdaType nodeSelector);

      MainWindow* m_mainWindow{ nullptr };

      const TreeNode<VizNode>* m_selectedNode{ nullptr };

      std::unique_ptr<VisualizationModel> m_treeMap{ nullptr };

      std::vector<const TreeNode<VizNode>*> m_highlightedNodes;

      VisualizationParameters m_visualizationParameters;
};

#endif // MAINMODEL_H
