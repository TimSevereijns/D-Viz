#include "visualization.h"

#include <boost/optional.hpp>

#include <algorithm>
#include <iostream>
#include <string>
#include <Qt3DCore/QRay3D>
#include <QRectF>

namespace
{
   const double EPSILON = 0.0001;

   /**
    * @brief DoRayAndPlaneIntersect calculates whether the specified ray hits the specified plane,
    * given a margin of error, epsilon.
    *
    * @param[in] ray                The ray to be fired at the plane.
    * @param[in] pointOnPlane       Any point on the plane.
    * @param[in] planeNormal        The normal for that point on the plane.
    *
    * @returns the point of intersection if there is an intersection greater than the margin of
    * error, or boost::none if no such intersection exists.
    */
   boost::optional<QVector3D> DoesRayIntersectPlane(const Qt3D::QRay3D& ray,
      const QVector3D& pointOnPlane, const QVector3D& planeNormal)
   {
      const double denominator = QVector3D::dotProduct(ray.direction(), planeNormal);
      if (std::abs(denominator) < EPSILON)
      {
         return false;
      }

      const double numerator = QVector3D::dotProduct(pointOnPlane - ray.origin(), planeNormal);

      const double scalar = numerator / denominator;
      const bool doesRayHitPlane = scalar > EPSILON;

      if (doesRayHitPlane)
      {
         return scalar * ray.direction() + ray.origin();
      }

      return boost::none;
   }

   /**
    * @brief DoesRayIntersectBLock
    *
    * @param[in] ray
    * @param[in] node
    *
    * @returns true if the ray intersects the block; false otherwise.
    */
   bool DoesRayIntersectBlock(const Qt3D::QRay3D& ray, const Block& block)
   {
      const bool intersectionFound = std::any_of(std::begin(block), std::end(block),
         [&ray] (const BlockFace& face)
      {
         if (face.side != BlockFace::Side::FRONT)
         {
            return false;
         }

         const QVector3D& randomPointOnFace = face.vertices[0];
         const QVector3D& normalForRandomPoint = face.vertices[1];

         const boost::optional<QVector3D> intersectionPoint =
            DoesRayIntersectPlane(ray, randomPointOnFace, normalForRandomPoint);

         if (!intersectionPoint)
         {
            return false;
         }

         std::cout << "Plane intersection found: " << intersectionPoint->x() << std::endl;

         return
            face.vertices[0].x() < intersectionPoint->x() &&
            face.vertices[2].x() > intersectionPoint->x() &&
            face.vertices[0].y() < intersectionPoint->y() &&
            face.vertices[4].y() > intersectionPoint->y();
      });

      return intersectionFound;
   }
}

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

void Visualization::PopulateVertexAndColorBuffers(const VisualizationParameters& parameters)
{
   assert(m_hasDataBeenParsed);
   assert(m_theTree);

   m_visualizationColors.clear();
   m_visualizationVertices.clear();

   if (!m_hasDataBeenParsed)
   {
      return;
   }

   int vertexCount = 0;

   std::for_each(m_theTree->beginPreOrder(), m_theTree->endPreOrder(),
      [&] (const TreeNode<VizNode>& node)
   {
      if ((parameters.onlyShowDirectories && node->file.type != FILE_TYPE::DIRECTORY) ||
          node->file.size < parameters.minimumFileSize)
      {
         return;
      }

      vertexCount = m_visualizationVertices.size();

      std::for_each(std::begin(node->block), std::end(node->block),
         [&] (const BlockFace& face)
      {
         m_visualizationVertices << face.vertices;
      });

      if (node->file.type == FILE_TYPE::DIRECTORY)
      {
         m_visualizationColors << Visualization::CreateDirectoryColors();
      }
      else if (node->file.type == FILE_TYPE::REGULAR)
      {
         m_visualizationColors << Visualization::CreateFileColors();
      }

      if (vertexCount + Block::VERTICES_PER_BLOCK != m_visualizationVertices.size())
      {
         std::cout << "Buffer data mismatch detected!" << std::endl;
      }
   });
}

QVector<QVector3D>& Visualization::GetColorBuffer()
{
   assert(!m_visualizationColors.empty());
   return m_visualizationColors;
}

double Visualization::ComputeNearestIntersection(const Qt3D::QRay3D& ray) const
{
   for (auto&& node : *m_theTree)
   {
      if (node->file.size < m_vizParameters.minimumFileSize)
      {
         continue;
      }

      const bool doesRayIntersectBlock = DoesRayIntersectBlock(ray, node->block);
      if (doesRayIntersectBlock)
      {
         std::cout << "Intersection found..." << std::endl;
      }
   }

   return std::numeric_limits<double>::infinity();
}

QVector<QVector3D>& Visualization::GetVertexBuffer()
{
   assert(!m_visualizationVertices.empty());
   return m_visualizationVertices;
}

QVector<QVector3D> Visualization::CreateFileColors()
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

void Visualization::SortNodes(Tree<VizNode>& tree)
{
   for (auto&& node : tree)
   {
      node.SortChildren([] (const TreeNode<VizNode>& lhs, const TreeNode<VizNode>& rhs)
         { return lhs->file.size > rhs->file.size; });
   }
}
