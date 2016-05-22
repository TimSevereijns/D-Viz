#include "visualization.h"

#include <boost/optional.hpp>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>

#include <Qt3DCore/QRay3D>
#include <QColor>
#include <QRectF>

#include "../constants.h"
#include "../ThirdParty/stopwatch.hpp"

namespace
{
   const double EPSILON = 0.0001;

   /**
    * @brief Calculates whether the specified ray hits the specified plane, given a margin of error,
    * epsilon.
    *
    * @param[in] ray                The ray to be fired at the plane.
    * @param[in] pointOnPlane       Any point on the plane.
    * @param[in] planeNormal        The normal for that point on the plane.
    *
    * @returns The point of intersection if there is an intersection greater than the margin of
    * error, or boost::none if no such intersection exists.
    */
   boost::optional<QVector3D> DoesRayIntersectPlane(
      const Qt3DCore::QRay3D& ray,
      const QVector3D& pointOnPlane,
      const QVector3D& planeNormal)
   {
      const double denominator = QVector3D::dotProduct(ray.direction(), planeNormal);
      if (std::abs(denominator) < EPSILON)
      {
         return boost::none;
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
    * @brief Returns the intersection point that is closest to the origin of the ray.
    *
    * @param[in] ray                The ray that caused the intersections.
    * @param[in] intersections      All the intersections caused by the ray.
    *
    * @return The closest intersection point, or boost::none should anything weird occur.
    */
   boost::optional<QVector3D> FindClosestIntersectionPoint(
      const Qt3DCore::QRay3D& ray,
      const std::vector<QVector3D>& allIntersections)
   {
      const auto& closest = std::min_element(std::begin(allIntersections), std::end(allIntersections),
         [&ray] (const auto& lhs, const auto& rhs) noexcept
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
    * @brief Finds the point at which the given ray intersects the given block.
    *
    * @param[in] ray                The ray fired at the block.
    * @param[in] block              The block to be tested for intersection.
    *
    * @returns The point of intersection should it exist; boost::none otherwise.
    */
   boost::optional<QVector3D> DoesRayIntersectBlock(
      const Qt3DCore::QRay3D& ray,
      const Block& block)
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
    * @brief Helper function that will advance the passed-in node to the next node in the tree that
    * is not a descendant of said node.
    *
    * @param[out] node              The node to advance.
    */
   void AdvanceToNextNonDescendant(TreeNode<VizNode>*& node)
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

   using IntersectionPointAndNode = std::pair<QVector3D, TreeNode<VizNode>*>;

   /**
    * @brief Iterates over all nodes in the scene, placing all intersections in a vector.
    *
    * @param[in] ray                The ray to be shot into the scene.
    * @param[in] camera             The camera from which the ray is shot.
    * @param[in] parameters         Additional visualization parameters.
    * @param[in] node               The current node being hit-tested.
    */
   std::vector<IntersectionPointAndNode> FindAllIntersections(
      const Qt3DCore::QRay3D& ray,
      const Camera& camera,
      const VisualizationParameters& parameters,
      TreeNode<VizNode>* node)
   {
      std::vector<IntersectionPointAndNode> allIntersections;

      assert(node);
      while (node)
      {
         if (node->GetData().file.size < parameters.minimumFileSize ||
             (parameters.onlyShowDirectories && node->GetData().file.type != FileType::DIRECTORY))
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
               allIntersections.emplace_back(std::make_pair(*blockIntersection, node));
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

const float Visualization::BLOCK_HEIGHT = 2.0f;
const float Visualization::ROOT_BLOCK_WIDTH = 1000.0f;
const float Visualization::ROOT_BLOCK_DEPTH = 1000.0f;

Visualization::Visualization(const VisualizationParameters& parameters) :
   m_vizParameters(parameters)
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
      [] (auto& node)
   {
      if (!node.HasChildren())
      {
         node->boundingBox = node->block;
         return;
      }

      double tallestDescendant = 0.0;

      auto* currentChild = node.GetFirstChild();
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
   assert(m_theTree);
   assert(m_hasDataBeenParsed);

   if (!m_hasDataBeenParsed)
   {
      return;
   }

   m_visualizationColors.clear();
   m_visualizationVertices.clear();

   std::for_each(m_theTree->beginPreOrder(), m_theTree->endPreOrder(),
      [&] (auto& node)
   {
      if ((parameters.onlyShowDirectories && node->file.type != FileType::DIRECTORY) ||
          node->file.size < parameters.minimumFileSize)
      {
         return;
      }

      const int vertexCount = m_visualizationVertices.size();
      node->offsetIntoVBO = vertexCount;

      std::for_each(std::begin(node->block), std::end(node->block),
         [&] (const auto& face)
      {
         m_visualizationVertices << face.vertices;
      });

      if (node->file.type == FileType::DIRECTORY)
      {
         if (parameters.useDirectoryGradient)
         {
            m_visualizationColors << ComputeGradientColor(node);
         }
         else
         {
            m_visualizationColors << Visualization::CreateDirectoryColors();
         }
      }
      else if (node->file.type == FileType::REGULAR)
      {
         m_visualizationColors << Visualization::CreateFileColors();
      }

      if (vertexCount + Block::VERTICES_PER_BLOCK != m_visualizationVertices.size())
      {
         assert(!"Buffer data mismatch detected!");
      }
   });

   // All offsets must be properly set; the default initialized state is invalid:
   assert(std::none_of(std::begin(*m_theTree), std::end(*m_theTree),
      [] (const auto& node) { return node->offsetIntoVBO == VizNode::INVALID_OFFSET; }));
}

TreeNode<VizNode>* Visualization::FindNearestIntersection(
   const Camera& camera,
   const Qt3DCore::QRay3D& ray,
   const VisualizationParameters& parameters) const
{
   if (!m_hasDataBeenParsed)
   {
      return nullptr;
   }

   TreeNode<VizNode>* nearestIntersection = nullptr;

   Stopwatch<std::chrono::milliseconds>([&]
   {
      auto allIntersections = FindAllIntersections(ray, camera, parameters, m_theTree->GetHead());
      if (allIntersections.empty())
      {
         return;
      }

      std::sort(std::begin(allIntersections), std::end(allIntersections),
         [&ray] (const IntersectionPointAndNode& lhs, const IntersectionPointAndNode& rhs) noexcept
      {
         return (ray.origin().distanceToPoint(lhs.first) < ray.origin().distanceToPoint(rhs.first));
      });

      nearestIntersection = allIntersections.front().second;
   }, "Node selected in ");

   return nearestIntersection;
}

void Visualization::FindSmallestandLargestDirectory(const Tree<VizNode>& tree)
{
   std::uintmax_t smallestDirectory = std::numeric_limits<std::uintmax_t>::max();
   std::uintmax_t largestDirectory = std::numeric_limits<std::uintmax_t>::min();

   for (auto& node : tree)
   {
      if (node.GetData().file.type != FileType::DIRECTORY)
      {
         continue;
      }

      const auto directorySize = node.GetData().file.size;

      if (directorySize < smallestDirectory)
      {
         smallestDirectory = directorySize;
      }
      else if (directorySize > largestDirectory)
      {
         largestDirectory = directorySize;
      }
   }

   m_largestDirectorySize = largestDirectory;
}

QVector<QVector3D>& Visualization::GetColorData()
{
   assert(!m_visualizationColors.empty());
   return m_visualizationColors;
}

QVector<QVector3D>& Visualization::GetVertexData()
{
   assert(!m_visualizationVertices.empty());
   return m_visualizationVertices;
}

QVector<QVector3D> Visualization::CreateFileColors()
{
   QVector<QVector3D> blockColors;
   blockColors.reserve(Block::VERTICES_PER_BLOCK);
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
   blockColors.reserve(Block::VERTICES_PER_BLOCK);
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

QVector<QVector3D> Visualization::ComputeGradientColor(const TreeNode<VizNode>& node)
{
   QVector<QVector3D> blockColors;
   blockColors.reserve(Block::VERTICES_PER_BLOCK);

   const auto blockSize = node.GetData().file.size;
   const auto ratio = static_cast<double>(blockSize) / static_cast<double>(m_largestDirectorySize);

   for (int i = 0; i < Block::VERTICES_PER_BLOCK; i++)
   {
      blockColors << m_directoryColorGradient.GetColorAtValue(static_cast<float>(ratio));
   }

   return blockColors;
}

QVector<QVector3D> Visualization::CreateHighlightColors()
{
   QVector<QVector3D> blockColors;
   blockColors.reserve(Block::VERTICES_PER_BLOCK);

   for (int i = 0; i < Block::VERTICES_PER_BLOCK; i++)
   {
      blockColors << Constants::Colors::CANARY_YELLOW;
   }

   return blockColors;
}

void Visualization::SortNodes(Tree<VizNode>& tree)
{
   for (auto& node : tree)
   {
      node.SortChildren(
         [] (const auto& lhs, const auto& rhs) noexcept
      {
         return lhs->file.size > rhs->file.size;
      });
   }
}
