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

Visualization::Visualization(const std::wstring& rawPath)
   : m_diskScanner(rawPath),
     m_hasDataBeenParsed(false),
     m_hasScanBeenPerformed(false)
{
}

Visualization::~Visualization()
{
}

void Visualization::ScanDirectory(std::function<void (const std::uintmax_t)> progressCallback)
{
   std::atomic<std::pair<std::uintmax_t, bool>> progress{std::make_pair(0, false)};
   m_diskScanner.ScanInNewThread(&progress);

   while (progress.load().second == false)
   {
      std::cout << "Files scanned so far: "
                << progress.load().first << std::endl;

      progressCallback(progress.load().first);

      std::this_thread::sleep_for(std::chrono::seconds(1));
   }

   m_diskScanner.JoinScanningThread();
   m_diskScanner.PrintTreeMetadata();

   m_hasScanBeenPerformed = true;
}

QVector<QVector3D>& Visualization::PopulateVertexBuffer(const ParsingOptions& options)
{
   assert(m_hasDataBeenParsed);

   m_visualizationVertices.clear();

   if (!m_hasDataBeenParsed)
   {
      return m_visualizationVertices;
   }

   const Tree<VizNode>& fileTree = m_diskScanner.GetFileTree();
   std::for_each(fileTree.beginPreOrder(), fileTree.endPreOrder(),
      [&] (const TreeNode<VizNode>& node)
   {
      if (options.showDirectoriesOnly && node.GetData().m_file.m_type == FILE_TYPE::DIRECTORY)
      {
         m_visualizationVertices << node.GetData().m_block.m_vertices;
      }
      else if (!options.showDirectoriesOnly)
      {
         m_visualizationVertices << node.GetData().m_block.m_vertices;
      }
   });

   std::cout << "Vertex count: " << m_visualizationVertices.size() << std::endl;
   std::cout << "Block count: " << m_visualizationVertices.size() / Block::VERTICES_PER_BLOCK << std::endl;

   return m_visualizationVertices;
}

unsigned int Visualization::GetVertexCount() const
{
   assert(m_hasDataBeenParsed);

   return m_visualizationVertices.size();
}

bool Visualization::HasScanBeenPerformed() const
{
   return m_hasScanBeenPerformed;
}

QVector<QVector3D>& Visualization::PopulateColorBuffer(const ParsingOptions& options)
{
   assert(m_hasDataBeenParsed);

   m_visualizationColors.clear();

   if (!m_hasDataBeenParsed)
   {
      return m_visualizationColors;
   }

   const Tree<VizNode>& fileTree = m_diskScanner.GetFileTree();
   std::for_each(fileTree.beginPreOrder(), fileTree.endPreOrder(),
      [&] (const TreeNode<VizNode>& node)
   {
      if (node.GetData().m_file.m_type == FILE_TYPE::DIRECTORY)
      {
         m_visualizationColors << Visualization::CreateDirectoryColors();
      }
      else if (node.GetData().m_file.m_type == FILE_TYPE::REGULAR && !options.showDirectoriesOnly)
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
