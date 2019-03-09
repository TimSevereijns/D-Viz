#ifndef SQUARIFIEDTREEMAP_H
#define SQUARIFIEDTREEMAP_H

#include "visualization.h"

/**
 * @brief Represents the Squarified tree map visualization.
 */
class SquarifiedTreeMap final : public VisualizationModel
{
  public:
    /**
     * @brief Constructs a new tree map laid out using the Squarified algorithm.
     *
     * @param[in] fileMonitor     The file monitoring implementation to use.
     * @param[in] path            The path representing the root directory of the tree map.
     */
    SquarifiedTreeMap(
        std::unique_ptr<FileMonitorBase> fileMonitor,
        const std::experimental::filesystem::path& path);

    /**
     * @brief Parses the file system tree into the model.
     *
     * @param[in] theTree         The filesystem tree to be parsed.
     */
    void Parse(const std::shared_ptr<Tree<VizBlock>>& theTree) override;

  private:
    /**
     * @brief Computes the area of the specified block that remains available to be built upon.
     *
     * @param[in] block              The block to build upon.
     */
    Block ComputeRemainingArea(const Block& block);

    /**
     * @brief Calculates the shortest dimension (width or depth) of the remaining bounds available
     * to build within.
     *
     * @param[in] node               The node being built upon.
     *
     * @returns A double respresent the length of the shortest edge.
     */
    double ComputeShortestEdgeOfRemainingBounds(const VizBlock& node);

    /**
     * @brief Calculates the worst aspect ratio of all items accepted into the row along with one
     * optional candidate item.
     *
     * @param[in] row                   The nodes that have been placed in the current real estate.
     * @param[in] candidateSize         The size of the candidate node that is to be considered
     *                                  for inclusion in the current row. Zero is no candidate
     *                                  necessary.
     * @param[in] shortestEdgeOfBounds  Length of shortest side of the enclosing row's boundary.
     *
     * @returns A double representing the least square aspect ratio.
     */
    double ComputeWorstAspectRatio(
        const std::vector<Tree<VizBlock>::Node*>& row, const std::uintmax_t candidateSize,
        VizBlock& parentNode, const double shortestEdgeOfBounds);

    /**
     * @brief Represents the heart of the algorithm and decides which nodes ought to be added to
     * which row in order to acheive an acceptable layout.
     *
     * @param[in, out] nodes         The sibling nodes to be laid out within the available bounds
     *                               of the parent node.
     */
    void SquarifyAndLayoutRows(const std::vector<Tree<VizBlock>::Node*>& nodes);

    /**
     * @brief The main entry point into the squarification algorithm, and performs a recursive
     * breadth-first traversal of the node tree and lays out the children of each node with the
     * aid of various helper functions.
     *
     * @param[in, out] root          The node whose children to lay out.
     */
    void SquarifyRecursively(const Tree<VizBlock>::Node& root);

    /**
     * @brief Computes the outer bounds (including the necessary boundary padding) needed to
     * properly
     * contain the row once laid out on top of its parent node.
     *
     * @param[in] bytesInRow         The total size of the row in bytes.
     * @param[in, out] parentNode    The node on top of which the new row is to be placed.
     * @param[in] updateOffset       Whether the origin of the next row should be computed. This
     *                               should only be set to true only when the row bounds are
     *                               computed for the last time as part of row layout.
     *
     * @returns A block representing the outer dimensions of the row boundary.
     */
    Block
    CalculateRowBounds(std::uintmax_t bytesInRow, VizBlock& parentNode, const bool updateOffset);

    /**
     * @brief Takes all the nodes that are to be included in a single row and then constructs the
     * individual blocks representing the nodes in that row so that the bounds of the row are
     * subdivided along the longest axis of available space.
     *
     * @param[in, out] row           The nodes to include in a single row.
     */
    void LayoutRow(std::vector<Tree<VizBlock>::Node*>& row);
};

#endif // SQUARIFIEDTREEMAP_H
