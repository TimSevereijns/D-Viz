#ifndef VISUALIZATIONASSET_H
#define VISUALIZATIONASSET_H

#include "baseAsset.h"

#include "../Utilities/colorGradient.hpp"

struct VizFile;

template<typename DataType>
class TreeNode;

namespace Asset
{
   /**
    * @brief The Treemap class implements the functionality needed to represent the main
    * visualization asset.
    */
   class Treemap final : public Base
   {
      public:

         explicit Treemap(QOpenGLExtraFunctions& openGL);

         /**
          * @see Asset::Base::LoadShaders(...)
          */
         bool LoadShaders() override;

         /**
          * @see Asset::Base::Initialize(...)
          */
         bool Initialize() override;

         /**
          * @see Asset::Base::Render(...)
          */
         bool Render(
            const Camera& camera,
            const std::vector<Light>& lights,
            const Settings::Manager& settings) override;

         /**
          * @see Asset::Base::Reload(...)
          */
         bool Reload() override;

         /**
          * @see Asset::Base::UpdateVBO(...)
          */
         void UpdateVBO(
            const Tree<VizFile>::Node& node,
            Asset::Event action,
            const Settings::Manager& settings) override;

         /**
          * @see Asset::Base::IsAssetLoaded(...)
          */
         bool IsAssetLoaded() const override;

         /**
          * @brief Loads the TreeMap nodes into the necessary graphics buffers.
          *
          * @note Passing the tree in as a const reference is a bit of a lie, since the nodes can
          * still be (and are) modifiable. Consider fixing this.
          *
          * @param[in] tree            The tree to pull the visualized TreeMap information from.
          * @param[in] settings        A reference to the settings manager so that the visualization
          *                            can take into account the current settings.
          *
          * @returns The number of blocks that have been loaded into the buffer.
          */
         std::uint32_t LoadBufferData(
            const Tree<VizFile>& tree,
            const Settings::Manager& settings);

         /**
          * @brief Reloads the color buffer without touching the other buffers.
          *
          * @note Calling this function without populating the corresponding block transformation
          * and reference block buffers is likely to have unintended consequences.
          *
          * @param[in] tree            The tree to pull the visualized TreeMap information from.
          * @param[in] settings        A reference to the settings manager so that the visualization
          *                            can take into account the current settings.
          */
         void ReloadColorBufferData(
            const Tree<VizFile>& tree,
            const Settings::Manager& settings);

         /**
          * @returns The number of blocks that are currently loaded into the visualization asset.
          */
         std::uint32_t GetBlockCount() const;

      private:

         QVector3D ComputeGradientColor(const Tree<VizFile>::Node& node);

         void ComputeAppropriateBlockColor(
            const Tree<VizFile>::Node& node,
            const Settings::Manager& settings);

         void FindLargestDirectory(const Tree<VizFile>& tree);

         bool InitializeReferenceBlock();
         bool InitializeColors();
         bool InitializeBlockTransformations();
         bool InitializeShadowMachinery();

         ColorGradient m_directoryColorGradient;

         std::uint32_t m_blockCount{ 0 };
         std::uintmax_t m_largestDirectorySize{ 0 };

         QOpenGLBuffer m_referenceBlockBuffer;
         QOpenGLBuffer m_blockTransformationBuffer;
         QOpenGLBuffer m_blockColorBuffer;

         QVector<QVector3D> m_referenceBlockVertices;
         QVector<QMatrix4x4> m_blockTransformations;
         QVector<QVector3D> m_blockColors;
   };
}

#endif // VISUALIZATIONASSET_H
