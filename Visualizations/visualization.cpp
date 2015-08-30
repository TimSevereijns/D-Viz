#include "visualization.h"

#include "../diskScanner.h"

#include <algorithm>
#include <iostream>
#include <string>

const double Visualization::BLOCK_HEIGHT = 2.0;
const double Visualization::PADDING_RATIO = 0.9;
const double Visualization::MAX_PADDING = 0.75;
const double Visualization::ROOT_BLOCK_WIDTH = 1000.0;
const double Visualization::ROOT_BLOCK_DEPTH = 1000.0;

Visualization::Visualization(const VisualizationParameters& parameters)
   : m_vizParameters(parameters),
     m_hasDataBeenParsed(false)
{
}

Visualization::~Visualization()
{
}

QVector<QVector3D>& Visualization::PopulateVertexBuffer(const VisualizationParameters& parameters)
{
   assert(m_hasDataBeenParsed);
   assert(m_theTree);

   m_visualizationVertices.clear();

   if (!m_hasDataBeenParsed)
   {
      return m_visualizationVertices;
   }

   std::for_each(m_theTree->beginPreOrder(), m_theTree->endPreOrder(),
      [&] (const TreeNode<VizNode>& node)
   {
      if (!parameters.onlyShowDirectories &&
         node.GetData().m_file.m_size >= parameters.minimumFileSize)
      {
         m_visualizationVertices << node.GetData().m_block.m_vertices;
      }
      else if (parameters.onlyShowDirectories &&
         node.GetData().m_file.m_type == FILE_TYPE::DIRECTORY &&
         node.GetData().m_file.m_size >= parameters.minimumFileSize)
      {
         m_visualizationVertices << node.GetData().m_block.m_vertices;
      }
   });

   std::cout << "Vertex count: " << m_visualizationVertices.size() << std::endl;
   std::cout << "Block count: " << m_visualizationVertices.size() / Block::VERTICES_PER_BLOCK << std::endl;

   return m_visualizationVertices;
}

QVector<QVector3D>& Visualization::PopulateColorBuffer(const VisualizationParameters& options)
{
   assert(m_hasDataBeenParsed);
   assert(m_theTree);

   m_visualizationColors.clear();

   if (!m_hasDataBeenParsed)
   {
      return m_visualizationColors;
   }

   std::for_each(m_theTree->beginPreOrder(), m_theTree->endPreOrder(),
      [&] (const TreeNode<VizNode>& node)
   {
      if (!options.onlyShowDirectories &&
         node.GetData().m_file.m_type == FILE_TYPE::REGULAR &&
         node.GetData().m_file.m_size >= options.minimumFileSize)
      {
         m_visualizationColors << Visualization::CreateBlockColors();
      }
      else if (node.GetData().m_file.m_type == FILE_TYPE::DIRECTORY &&
         node.GetData().m_file.m_size >= options.minimumFileSize)
      {
         m_visualizationColors << Visualization::CreateDirectoryColors();
      }
   });

   std::cout << "Color count: " << m_visualizationColors.size() / 30 << std::endl;

   return m_visualizationColors;
}

unsigned int Visualization::GetVertexCount() const
{
   assert(m_hasDataBeenParsed);

   return m_visualizationVertices.size();
}

QVector<QVector3D> Visualization::CreateBlockColors()
{
   QVector<QVector3D> blockColors;
   blockColors.reserve(30);
   blockColors
      << QVector3D(1, 0, 0) << QVector3D(1, 0, 0) << QVector3D(1, 0, 0) // Front
      << QVector3D(1, 0, 0) << QVector3D(1, 0, 0) << QVector3D(1, 0, 0)
      << QVector3D(0, 1, 0) << QVector3D(0, 1, 0) << QVector3D(0, 1, 0) // Right
      << QVector3D(0, 1, 0) << QVector3D(0, 1, 0) << QVector3D(0, 1, 0)
      << QVector3D(1, 0, 0) << QVector3D(1, 0, 0) << QVector3D(1, 0, 0) // Back
      << QVector3D(1, 0, 0) << QVector3D(1, 0, 0) << QVector3D(1, 0, 0)
      << QVector3D(0, 1, 0) << QVector3D(0, 1, 0) << QVector3D(0, 1, 0) // Left
      << QVector3D(0, 1, 0) << QVector3D(0, 1, 0) << QVector3D(0, 1, 0)
      << QVector3D(0, 0, 1) << QVector3D(0, 0, 1) << QVector3D(0, 0, 1) // Top
      << QVector3D(0, 0, 1) << QVector3D(0, 0, 1) << QVector3D(0, 0, 1);

   return blockColors;
}

QVector<QVector3D> Visualization::CreateDirectoryColors()
{
   QVector<QVector3D> blockColors;
   blockColors.reserve(30);
   blockColors
      << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) // Front
      << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) << QVector3D(1, 1, 1)
      << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) // Right
      << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) << QVector3D(1, 1, 1)
      << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) // Back
      << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) << QVector3D(1, 1, 1)
      << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) // Left
      << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) << QVector3D(1, 1, 1)
      << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) // Top
      << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) << QVector3D(1, 1, 1);

   return blockColors;
}
