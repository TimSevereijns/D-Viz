#include "visualization.h"

#include <boost/optional.hpp>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include <Qt3DCore/QRay3D>
#include <QRectF>

#include "../Utilities/stopwatch.hpp"

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
      const bool doesRayHitPlane = std::abs(scalar) > EPSILON;

      if (doesRayHitPlane)
      {
         return scalar * ray.direction().normalized() + ray.origin();
      }

      return boost::none;
   }

   /**
    * @brief FindClosestIntersectionPoint will return the intersection point that is closest to the
    * origin of the ray.
    *
    * @param[in] ray                The ray that caused the intersections.
    * @param[in] intersections      All the intersections caused by the ray.
    *
    * @return the closest intersection point, or boost::none should anything weird occur.
    */
   boost::optional<QVector3D> FindClosestIntersectionPoint(const Qt3D::QRay3D& ray,
      const std::vector<QVector3D>& allIntersections)
   {
      const auto& closest = std::min_element(std::begin(allIntersections), std::end(allIntersections),
         [&ray] (const QVector3D& lhs, const QVector3D& rhs)
      {
         return (ray.origin().distanceToPoint(lhs) < ray.origin().distanceToPoint(rhs));
      });

      if (closest != std::end(allIntersections))
      {
         return *closest;
      }

      return boost::none;
   }

   /**
    * @brief DoesRayIntersectBlock will find the point at which the given ray intersects the given
    * block.
    *
    * @param[in] ray                The ray fired at the block.
    * @param[in] block              The block to be tested for intersection.
    *
    * @returns the point of intersection should it exist; boost::none otherwise.
    */
   boost::optional<QVector3D> DoesRayIntersectBlock(const Qt3D::QRay3D& ray, const Block& block)
   {
      std::vector<QVector3D> allIntersections;

      std::for_each(std::begin(block), std::end(block),
         [&ray, &allIntersections] (const BlockFace& face)
      {
         const QVector3D& randomPointOnFace = face.vertices[0];
         const QVector3D& normalForRandomPoint = face.vertices[1];

         const boost::optional<QVector3D> intersectionPoint =
            DoesRayIntersectPlane(ray, randomPointOnFace, normalForRandomPoint);

         if (!intersectionPoint)
         {
            return;
         }

         if (face.side == BlockFace::Side::TOP)
         {
            if (face.vertices[0].x() < intersectionPoint->x() &&
                face.vertices[2].x() > intersectionPoint->x() &&
                face.vertices[0].z() > intersectionPoint->z() &&
                face.vertices[4].z() < intersectionPoint->z())
            {
               allIntersections.emplace_back(*intersectionPoint);
            }
         }
         else if (face.side == BlockFace::Side::FRONT)
         {
            if (face.vertices[0].x() < intersectionPoint->x() &&
                face.vertices[2].x() > intersectionPoint->x() &&
                face.vertices[0].y() < intersectionPoint->y() &&
                face.vertices[4].y() > intersectionPoint->y())
            {
               allIntersections.emplace_back(*intersectionPoint);
            }
         }
         else if (face.side == BlockFace::Side::BACK)
         {
            if (face.vertices[2].x() < intersectionPoint->x() &&
                face.vertices[0].x() > intersectionPoint->x() &&
                face.vertices[2].y() < intersectionPoint->y() &&
                face.vertices[6].y() > intersectionPoint->y())
            {
               allIntersections.emplace_back(*intersectionPoint);
            }
         }
         else if (face.side == BlockFace::Side::LEFT)
         {
            if (face.vertices[2].z() > intersectionPoint->z() &&
                face.vertices[0].z() < intersectionPoint->z() &&
                face.vertices[0].y() < intersectionPoint->y() &&
                face.vertices[4].y() > intersectionPoint->y())
            {
               allIntersections.emplace_back(*intersectionPoint);
            }
         }
         else if (face.side == BlockFace::Side::RIGHT)
         {
            if (face.vertices[0].z() > intersectionPoint->z() &&
                face.vertices[2].z() < intersectionPoint->z() &&
                face.vertices[0].y() < intersectionPoint->y() &&
                face.vertices[4].y() > intersectionPoint->y())
            {
               allIntersections.emplace_back(*intersectionPoint);
            }
         }
      });

      return FindClosestIntersectionPoint(ray, allIntersections);
   }

   /**
    * @brief AdvanceToNextNonDescendant is a helper function that will advance the passed-in node
    * to the next node in the tree that is not a descendant of said node.
    *
    * @param[out] node              The node to advance.
    */
   void AdvanceToNextNonDescendant(std::shared_ptr<TreeNode<VizNode>>& node)
   {
      if (node->GetNextSibling())
      {
         node = node->GetNextSibling();
      }
      else
      {
         while (node->GetParent() && !node->GetParent()->GetNextSibling())
         {
            node = node->GetParent();
         }

         if (node->GetParent())
         {
            node = node->GetParent()->GetNextSibling();
         }
         else
         {
            node = nullptr;
         }
      }
   }

   using IntersectionPointAndNode = std::pair<QVector3D, TreeNode<VizNode>>;

   /**
    * @brief GetAllIntersections
    *
    * @param[in] ray
    * @param[in] camera
    * @param[in] parameters
    * @param[in] node
    */
   std::vector<IntersectionPointAndNode> FindAllIntersections(
      const Qt3D::QRay3D& ray,
      const Camera& camera,
      const VisualizationParameters& parameters,
      std::shared_ptr<TreeNode<VizNode>> node)
   {
      std::vector<IntersectionPointAndNode> allIntersections;

      assert(node);
      while (node)
      {
         if (node->GetData().file.size < parameters.minimumFileSize ||
             (parameters.onlyShowDirectories && node->GetData().file.type != FILE_TYPE::DIRECTORY))
         {
            AdvanceToNextNonDescendant(node);
            continue;
         }

         const auto& boundingBoxIntersection = DoesRayIntersectBlock(ray, node->GetData().boundingBox);
         if (boundingBoxIntersection)
         {
            const auto& blockIntersection = DoesRayIntersectBlock(ray, node->GetData().block);
            if (blockIntersection && camera.IsPointInFrontOfCamera(*blockIntersection))
            {
               allIntersections.emplace_back(std::make_pair(*blockIntersection, *node));
            }

            if (node->HasChildren())
            {
               node = node->GetFirstChild();
            }
            else
            {
               AdvanceToNextNonDescendant(node);
            }
         }
         else
         {
            AdvanceToNextNonDescendant(node);
         }
      }

      return allIntersections;
   }
}

const double Visualization::PADDING_RATIO = 0.9;
const double Visualization::MAX_PADDING = 0.75;

const float Visualization::BLOCK_HEIGHT = 2.0;
const float Visualization::ROOT_BLOCK_WIDTH = 1000.0;
const float Visualization::ROOT_BLOCK_DEPTH = 1000.0;

Visualization::Visualization(const VisualizationParameters& parameters)
   : m_vizParameters(parameters),
     m_theTree(nullptr),
     m_hasDataBeenParsed(false)
{
}

Visualization::~Visualization()
{
}

void Visualization::UpdateBoundingBoxes()
{
   assert(m_hasDataBeenParsed);
   assert(m_theTree);

   if (!m_hasDataBeenParsed)
   {
      return;
   }

   std::for_each(std::begin(*m_theTree), std::end(*m_theTree),
      [&] (TreeNode<VizNode>& node)
   {
      if (!node.HasChildren())
      {
         node->boundingBox = node->block;
         return;
      }

      double tallestDescendant = 0.0;

      std::shared_ptr<TreeNode<VizNode>> currentChild = node.GetFirstChild();
      while (currentChild)
      {
         if (currentChild->GetData().boundingBox.height > tallestDescendant)
         {
            tallestDescendant = currentChild->GetData().boundingBox.height;
         }

         currentChild = currentChild->GetNextSibling();
      }

      node->boundingBox = Block
      {
         node->block.origin,
         node->block.width,
         node->block.height + tallestDescendant,
         node->block.depth
      };
   });
}

void Visualization::ComputeVertexAndColorData(const VisualizationParameters& parameters)
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

QVector<QVector3D>& Visualization::GetColorData()
{
   assert(!m_visualizationColors.empty());
   return m_visualizationColors;
}

boost::optional<TreeNode<VizNode>> Visualization::FindNearestIntersection(
   const Camera& camera,
   const Qt3D::QRay3D& ray,
   const VisualizationParameters& parameters) const
{
   if (!m_hasDataBeenParsed)
   {
      return boost::none;
   }

   boost::optional<TreeNode<VizNode>> nearestIntersection = boost::none;

   Stopwatch<std::chrono::milliseconds>([&]{
      auto allIntersections = FindAllIntersections(ray, camera, parameters, m_theTree->GetHead());

      if (allIntersections.empty())
      {
         return;
      }

      std::sort(std::begin(allIntersections), std::end(allIntersections),
         [&ray] (const IntersectionPointAndNode& lhs, const IntersectionPointAndNode& rhs)
      {
         return (ray.origin().distanceToPoint(lhs.first) < ray.origin().distanceToPoint(rhs.first));
      });

      nearestIntersection = allIntersections.front().second;
   }, "Node selected in ");

   return nearestIntersection;
}

QVector<QVector3D>& Visualization::GetVertexData()
{
   assert(!m_visualizationVertices.empty());
   return m_visualizationVertices;
}

QVector<QVector3D> Visualization::CreateFileColors()
{
   QVector<QVector3D> blockColors;
   blockColors.reserve(30);
   blockColors
      // Front:
      << QVector3D(1.0f, 0.0f, 0.0f) << QVector3D(1.0f, 0.0f, 0.0f) << QVector3D(1.0f, 0.0f, 0.0f)
      << QVector3D(1.0f, 0.0f, 0.0f) << QVector3D(1.0f, 0.0f, 0.0f) << QVector3D(1.0f, 0.0f, 0.0f)
      // Right:
      << QVector3D(0.0f, 1.0f, 0.0f) << QVector3D(0.0f, 1.0f, 0.0f) << QVector3D(0.0f, 1.0f, 0.0f)
      << QVector3D(0.0f, 1.0f, 0.0f) << QVector3D(0.0f, 1.0f, 0.0f) << QVector3D(0.0f, 1.0f, 0.0f)
      // Back:
      << QVector3D(1.0f, 0.0f, 0.0f) << QVector3D(1.0f, 0.0f, 0.0f) << QVector3D(1.0f, 0.0f, 0.0f)
      << QVector3D(1.0f, 0.0f, 0.0f) << QVector3D(1.0f, 0.0f, 0.0f) << QVector3D(1.0f, 0.0f, 0.0f)
      // Left:
      << QVector3D(0.0f, 1.0f, 0.0f) << QVector3D(0.0f, 1.0f, 0.0f) << QVector3D(0.0f, 1.0f, 0.0f)
      << QVector3D(0.0f, 1.0f, 0.0f) << QVector3D(0.0f, 1.0f, 0.0f) << QVector3D(0.0f, 1.0f, 0.0f)
      // Top:
      << QVector3D(0.0f, 0.0f, 1.0f) << QVector3D(0.0f, 0.0f, 1.0f) << QVector3D(0.0f, 0.0f, 1.0f)
      << QVector3D(0.0f, 0.0f, 1.0f) << QVector3D(0.0f, 0.0f, 1.0f) << QVector3D(0.0f, 0.0f, 1.0f);

   return blockColors;
}

QVector<QVector3D> Visualization::CreateDirectoryColors()
{
   QVector<QVector3D> blockColors;
   blockColors.reserve(30);
   blockColors
      // Front:
      << QVector3D(1.0f, 1.0f, 1.0f) << QVector3D(1.0f, 1.0f, 1.0f) << QVector3D(1.0f, 1.0f, 1.0f)
      << QVector3D(1.0f, 1.0f, 1.0f) << QVector3D(1.0f, 1.0f, 1.0f) << QVector3D(1.0f, 1.0f, 1.0f)
      // Right:
      << QVector3D(1.0f, 1.0f, 1.0f) << QVector3D(1.0f, 1.0f, 1.0f) << QVector3D(1.0f, 1.0f, 1.0f)
      << QVector3D(1.0f, 1.0f, 1.0f) << QVector3D(1.0f, 1.0f, 1.0f) << QVector3D(1.0f, 1.0f, 1.0f)
      // Back:
      << QVector3D(1.0f, 1.0f, 1.0f) << QVector3D(1.0f, 1.0f, 1.0f) << QVector3D(1.0f, 1.0f, 1.0f)
      << QVector3D(1.0f, 1.0f, 1.0f) << QVector3D(1.0f, 1.0f, 1.0f) << QVector3D(1.0f, 1.0f, 1.0f)
      // Left:
      << QVector3D(1.0f, 1.0f, 1.0f) << QVector3D(1.0f, 1.0f, 1.0f) << QVector3D(1.0f, 1.0f, 1.0f)
      << QVector3D(1.0f, 1.0f, 1.0f) << QVector3D(1.0f, 1.0f, 1.0f) << QVector3D(1.0f, 1.0f, 1.0f)
      // Top:
      << QVector3D(1.0f, 1.0f, 1.0f) << QVector3D(1.0f, 1.0f, 1.0f) << QVector3D(1.0f, 1.0f, 1.0f)
      << QVector3D(1.0f, 1.0f, 1.0f) << QVector3D(1.0f, 1.0f, 1.0f) << QVector3D(1.0f, 1.0f, 1.0f);

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
