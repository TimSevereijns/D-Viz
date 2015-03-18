#include "visualization.h"

#include <algorithm>
#include <iostream>
#include <string>

const float Visualization::BLOCK_HEIGHT = 0.0625f;
const float Visualization::BLOCK_TO_REAL_ESTATE_RATIO = 0.9f;

Visualization::Visualization(const std::wstring& rawPath)
   : m_diskScanner(rawPath),
     m_hasDataBeenParsed(false)
{
}

Visualization::~Visualization()
{
}

void Visualization::ScanDirectory()
{
   std::atomic<std::pair<std::uintmax_t, bool>> progress{std::make_pair(0, false)};
   m_diskScanner.ScanInNewThread(&progress);

   while (progress.load().second == false)
   {
      std::cout << "Files scanned so far: "
                << progress.load().first << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(1));
   }

   m_diskScanner.JoinScanningThread();
   m_diskScanner.PrintTreeMetadata();
   //m_diskScanner.PrintTree();
}

QVector<QVector3D>& Visualization::GetVertices()
{
   assert(m_hasDataBeenParsed);

   m_visualizationVertices.clear();

   if (!m_hasDataBeenParsed)
   {
      return m_visualizationVertices;
   }

   const Tree<VizNode>& directoryTree = m_diskScanner.GetDirectoryTree();
   std::for_each(directoryTree.beginPreOrder(), directoryTree.endPreOrder(),
      [&] (const TreeNode<VizNode>& node)
   {
      m_visualizationVertices << node.GetData().m_block.m_vertices;
   });

   std::cout << "Vertex count: " << m_visualizationVertices.size() << std::endl;
   std::cout << "Block count: " << m_visualizationVertices.size() / 60 << std::endl;

   return m_visualizationVertices;
}

QVector<QVector3D>& Visualization::GetColors()
{
   assert(m_hasDataBeenParsed);

   m_visualizationColors.clear();

   if (!m_hasDataBeenParsed)
   {
      return m_visualizationColors;
   }

   const Tree<VizNode>& directoryTree = m_diskScanner.GetDirectoryTree();
   std::for_each(directoryTree.beginPreOrder(), directoryTree.endPreOrder(),
      [&] (const TreeNode<VizNode>& node)
   {
      if (node.GetData().m_file.m_type == FILE_TYPE::DIRECTORY)
      {
         m_visualizationColors << Visualization::CreateDirectoryColors();
      }
      else if (node.GetData().m_file.m_type == FILE_TYPE::REGULAR)
      {
         m_visualizationColors << Visualization::CreateBlockColors();
      }
   });

   std::cout << "Color count: " << m_visualizationColors.size() / 30 << std::endl;

   return m_visualizationColors;
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
/*      << QVector3D(0, 0, 1) << QVector3D(0, 0, 1) << QVector3D(0, 0, 1) // Bottom
      << QVector3D(0, 0, 1) << QVector3D(0, 0, 1) << QVector3D(0, 0, 1)*/;

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
//      << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) // Bottom
//      << QVector3D(1, 1, 1) << QVector3D(1, 1, 1) << QVector3D(1, 1, 1);

   return blockColors;
}
