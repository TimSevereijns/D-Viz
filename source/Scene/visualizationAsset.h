#ifndef VISUALIZATIONASSET_H
#define VISUALIZATIONASSET_H

#include "sceneAsset.h"

#include "../Utilities/colorGradient.hpp"

#include <QOpenGLTexture>

struct VizFile;
template<typename DataType> class TreeNode;

/**
 * @brief The VisualizationAsset class implements the functionality needed to represent the
 * main visualization scene asset.
 */
class VisualizationAsset final : public SceneAsset
{
   public:

      VisualizationAsset(GraphicsDevice& device);

      bool LoadShaders() override;

      bool Initialize() override;

      bool Render(
         const Camera& camera,
         const std::vector<Light>& lights,
         const OptionsManager& settings) override;

      bool Reload() override;

      void UpdateVBO(
         const Tree<VizFile>::Node& node,
         UpdateAction action,
         const VisualizationParameters& options) override;

      bool IsAssetLoaded() const override;

      /**
       * @brief Loads the TreeMap nodes into the necessary graphics buffers.
       *
       * @todo Passing the tree in as a const reference is a bit of a lie, since the nodes can still
       * be (and are) modifiable. Consider fixing this.
       *
       * @param[in] tree            The tree to pull the visualized TreeMap information from.
       * @param[in] parameters      The current visualization parameters that will govern what nodes
       *                            will be visualized.
       *
       * @returns The number of blocks that have been loaded into the buffer.
       */
      std::uint32_t LoadBufferData(
         const Tree<VizFile>& tree,
         const VisualizationParameters& parameters);

      /**
       * @returns The number of blocks that are currently loaded into the visualization asset.
       */
      std::uint32_t GetBlockCount() const;

   private:

      bool RenderShadowPass(const Camera& camera);

      QVector3D ComputeGradientColor(const Tree<VizFile>::Node& node);

      void FindLargestDirectory(const Tree<VizFile>& tree);

      ColorGradient m_directoryColorGradient;

      std::uint32_t m_blockCount{ 0 };
      std::uintmax_t m_largestDirectorySize{ 0 };

      bool InitializeReferenceBlock();
      bool InitializeColors();
      bool InitializeBlockTransformations();
      bool InitializeShadowMachinery();

      QOpenGLBuffer m_referenceBlockBuffer;
      QOpenGLBuffer m_blockTransformationBuffer;
      QOpenGLBuffer m_blockColorBuffer;

      QVector<QVector3D> m_referenceBlockVertices;
      QVector<QMatrix4x4> m_blockTransformations;
      QVector<QVector3D> m_blockColors;
};

#endif // VISUALIZATIONASSET_H
