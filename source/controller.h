#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "DataStructs/light.h"
#include "DataStructs/vizFile.h"
#include "DriveScanner/driveScanner.h"
#include <Tree/Tree.hpp>

#include <memory>
#include <vector>

#include <QVector3D>

struct ScanningProgress;

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
       */
      void ResetVisualization();

      /**
       * @returns A pointer to the selected node.
       */
      const Tree<VizFile>::Node* GetSelectedNode() const;

      /**
       * @returns A reference to the tree that represents the most recent drive scan.
       */
      Tree<VizFile>& GetTree();

      /**
       * @overload
       */
      const Tree<VizFile>& GetTree() const;

      /**
       * @returns A reference to the currently highlighted nodes. Highlighted nodes are distinct
       * from the selected node (of which there can be only one).
       */
      const std::vector<const Tree<VizFile>::Node*>& GetHighlightedNodes() const;

      /**
       * @brief Parses the drive scan results.
       *
       * @param[in] results         The scan results to be parsed.
       */
      void ParseResults(const std::shared_ptr<Tree<VizFile>>& results);

      /**
       * @brief Updates the bounding boxes for the current treemap.
       */
      void UpdateBoundingBoxes();

      /**
       * @brief Searches the treemap for the search query contained within the search box.
       *
       * @param[in] searchQuery              String to search against.
       * @param[in] deselectionCallback      UI callback to clear selection highlights.
       * @param[in] selectionCallback        UI callback to highlight matching nodes on the canvas.
       * @param[in] shouldSearchFiles        Pass in true to search files.
       * @param[in] shouldSearchDirectories  Pass in true to search directories.
       */
      void SearchTreeMap(
         const std::wstring& searchQuery,
         const std::function<void (std::vector<const Tree<VizFile>::Node*>&)>& deselectionCallback,
         const std::function<void (std::vector<const Tree<VizFile>::Node*>&)>& selectionCallback,
         bool shouldSearchFiles,
         bool shouldSearchDirectories);

      /**
       * @brief Highlights all nodes in the tree whose extension matches that of the passed in node.
       *
       * @param[in] sampleNode      The node whose extension is to be highlighted.
       * @param[in] callback        UI callback to highlight matching nodes on the canvas.
       */
      void HighlightAllMatchingExtensions(
         const Tree<VizFile>::Node& sampleNode,
         const std::function<void (std::vector<const Tree<VizFile>::Node*>&)>& callback);

      /**
       * @brief Highlights all nodes that descendant from the passed in node.
       *
       * @param[in] node            The node whose descendants to highlight.
       * @param[in] callback        UI callback to highlight matching nodes on the canvas.
       */
      void HighlightDescendants(
         const Tree<VizFile>::Node& node,
         const std::function<void (std::vector<const Tree<VizFile>::Node*>&)>& callback);

      /**
       * @brief Highlights all nodes that are ancestors of the passed in node.
       *
       * @param[in] node            The node whose ancestors are to be highlighted.
       * @param[in] callback        UI callback to highlight matching nodes on the canvas.
       */
      void HighlightAncestors(
         const Tree<VizFile>::Node& node,
         const std::function<void (std::vector<const Tree<VizFile>::Node*>&)>& callback);

      /**
       * @brief Clears the selected node, and restores the color of that selected node back to its
       * unselected color.
       */
      void ClearSelectedNode();

      /**
       * @brief Clears all highlighted nodes, and restores the color of any highlighted nodes back
       * to its unhighlighted color.
       *
       * @param[in] callback        UI callback to highlight matching nodes on the canvas.
       * @param[in] clearSelected   Pass in true if the primary selection target is to be cleared.
       */
      void ClearHighlightedNodes(
         const std::function<void (std::vector<const Tree<VizFile>::Node*>&)>& callback,
         bool clearSelected);

      /**
       * @brief Selects the passed in node.
       *
       * @param[in] node               A pointer to the node to be selected.
       * @param[in] selectorCallback   UI callback to highlight matching node on the canvas.
       */
      void SelectNodeAndUpdateStatusBar(
         const Tree<VizFile>::Node* const node,
         const std::function<void (const Tree<VizFile>::Node* const)>& selectorCallback);

      /**
       * @brief Uses the passed in ray to select the nearest node from the perspective of the
       * camera.
       *
       * @todo Remove the camera object, and pass in the camera's position instead.
       *
       * @param[in] camera              The camera from which the ray was shot.
       * @param[in] ray                 The picking ray.
       * @param[in] deselectionCallback UI callback to clear the canvas of selection highlights.
       * @param[in] selectionCallback   UI callback to highlight matching nodes on the canvas.
       */
      void SelectNodeViaRay(
         const Camera& camera,
         const Qt3DRender::RayCasting::QRay3D& ray,
         const std::function<void (std::vector<const Tree<VizFile>::Node*>&)>& deselectionCallback,
         const std::function<void (const Tree<VizFile>::Node* const)>& selectionCallback);

      /**
       * @brief Helper function to print visualization metadata to the bottom status bar.
       */
      void PrintMetadataToStatusBar();

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
         std::uintmax_t sizeInBytes);

      /**
       * @brief Computes the absolute file path of the selected node by traveling up tree.
       *
       * @param[in] node            The selected node.
       *
       * @returns The absolute file path.
       */
      static std::wstring ResolveCompleteFilePath(const Tree<VizFile>::Node& node);

      /**
       * @brief Prints selection details to the main window's status bar.
       */
      void PrintSelectionDetailsToStatusBar();

      /**
       * @brief Whether to allow the user to interact with the UI.
       *
       * @param allowInteraction    The new state.
       */
      void AllowUserInteractionWithModel(bool allowInteraction);

      /**
       * @returns True if the user is allow to select or highlight nodes in the visualization model.
       */
      bool IsUserAllowedToInteractWithModel() const;

      /**
       * @brief Saves the results of the scan.
       *
       * @param[in] progress        @see ScanningProgress
       */
      void SaveScanResults(const ScanningProgress& progress);

   private:

      template<typename NodeSelectorType>
      void ProcessSelection(
         const NodeSelectorType& nodeSelector,
         const std::function<void (std::vector<const Tree<VizFile>::Node*>&)>& callback);

      bool m_allowInteractionWithModel{ false };

      MainWindow* m_mainWindow{ nullptr };

      const Tree<VizFile>::Node* m_selectedNode{ nullptr };

      std::unique_ptr<VisualizationModel> m_treeMap{ nullptr };

      std::vector<const Tree<VizFile>::Node*> m_highlightedNodes;

      // @todo Move these onto the VizualizationModel class:
      std::uintmax_t m_filesInCurrentVisualization{ 0 };
      std::uintmax_t m_directoriesInCurrentVisualization{ 0 };
      std::uintmax_t m_totalBytesInCurrentVisualization{ 0 };
};

#endif // CONTROLLER_H
