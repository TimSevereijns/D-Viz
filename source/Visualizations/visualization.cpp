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
   constexpr double EPSILON = 0.0001;

   const QVector3D POSITIVE_X_NORMAL{  1,  0,  0 };
   const QVector3D POSITIVE_Y_NORMAL{  0,  1,  0 };
   const QVector3D POSITIVE_Z_NORMAL{  0,  0,  1 };
   const QVector3D NEGATIVE_X_NORMAL{ -1,  0,  0 };
   const QVector3D NEGATIVE_Y_NORMAL{  0, -1,  0 };
   const QVector3D NEGATIVE_Z_NORMAL{  0,  0, -1 };

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

      if (!doesRayHitPlane)
      {
         return boost::none;
      }

      return scalar * ray.direction().normalized() + ray.origin();
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

      if (closest == std::end(allIntersections))
      {
         return boost::none;
      }

      return *closest;
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
      // @todo Replace with a stack allocated container.
      std::vector<QVector3D> allIntersections;

      const auto blockOrigin = block.GetOrigin();
      const auto blockHeight = block.GetHeight();
      const auto blockWidth = block.GetWidth();
      const auto blockDepth = block.GetDepth();

      { // Perform hit detection on the top face:
         const auto randomPointOnTopFace = blockOrigin + DoublePoint3D{ 0, blockHeight, 0 };
         const QVector3D topFacePoint
         {
            static_cast<float>(randomPointOnTopFace.x()),
            static_cast<float>(randomPointOnTopFace.y()),
            static_cast<float>(randomPointOnTopFace.z())
         };

         const boost::optional<QVector3D> intersectionPoint =
            DoesRayIntersectPlane(ray, topFacePoint, POSITIVE_Y_NORMAL);

         if (!intersectionPoint)
         {
            return boost::none;
         }

         if (blockOrigin.x()                 < intersectionPoint->x() &&
             blockOrigin.x() + blockWidth    > intersectionPoint->x() &&
             blockOrigin.z()                 > intersectionPoint->z() &&
             blockOrigin.z() - blockDepth    < intersectionPoint->z())
         {
            allIntersections.emplace_back(*intersectionPoint);
         }
      }

      { // Perform hit detection on the front face:
         const auto randomPointOnFrontFace = blockOrigin;
         const QVector3D frontFacePoint
         {
            static_cast<float>(randomPointOnFrontFace.x()),
            static_cast<float>(randomPointOnFrontFace.y()),
            static_cast<float>(randomPointOnFrontFace.z())
         };

         const boost::optional<QVector3D> intersectionPoint =
            DoesRayIntersectPlane(ray, frontFacePoint, POSITIVE_Z_NORMAL);

         if (!intersectionPoint)
         {
            return boost::none;
         }

         if (blockOrigin.x()                 < intersectionPoint->x() &&
             blockOrigin.x() + blockWidth    > intersectionPoint->x() &&
             blockOrigin.y()                 < intersectionPoint->y() &&
             blockOrigin.y() + blockHeight   > intersectionPoint->y())
         {
            allIntersections.emplace_back(*intersectionPoint);
         }
      }

      { // Perform hit detection on the back face:
         const auto randomPointOnBackFace = blockOrigin + DoublePoint3D{ 0, 0, -blockDepth};
         const QVector3D backFacePoint
         {
            static_cast<float>(randomPointOnBackFace.x()),
            static_cast<float>(randomPointOnBackFace.y()),
            static_cast<float>(randomPointOnBackFace.z())
         };

         const boost::optional<QVector3D> intersectionPoint =
            DoesRayIntersectPlane(ray, backFacePoint, NEGATIVE_Z_NORMAL);

         if (!intersectionPoint)
         {
            return boost::none;
         }

         if (blockOrigin.x()                 < intersectionPoint->x() &&
             blockOrigin.x() + blockWidth    > intersectionPoint->x() &&
             blockOrigin.y()                 < intersectionPoint->y() &&
             blockOrigin.y() + blockHeight   > intersectionPoint->y())
         {
            allIntersections.emplace_back(*intersectionPoint);
         }
      }

      { // Perform hit detection on the left face:
         const auto randomPointOnLeftFace = blockOrigin;
         const QVector3D leftFacePoint
         {
            static_cast<float>(randomPointOnLeftFace.x()),
            static_cast<float>(randomPointOnLeftFace.y()),
            static_cast<float>(randomPointOnLeftFace.z())
         };

         const boost::optional<QVector3D> intersectionPoint =
            DoesRayIntersectPlane(ray, leftFacePoint, NEGATIVE_X_NORMAL);

         if (!intersectionPoint)
         {
            return boost::none;
         }

         if (blockOrigin.z()                 > intersectionPoint->z() &&
             blockOrigin.z() - blockDepth    < intersectionPoint->z() &&
             blockOrigin.y()                 < intersectionPoint->y() &&
             blockOrigin.y() + blockHeight   > intersectionPoint->y())
         {
            allIntersections.emplace_back(*intersectionPoint);
         }
      }

      { // Perform hit detection on the right face:
         const auto randomPointOnRightFace = blockOrigin + DoublePoint3D{ blockWidth, 0, 0 };
         const QVector3D rightFacePoint
         {
            static_cast<float>(randomPointOnRightFace.x()),
            static_cast<float>(randomPointOnRightFace.y()),
            static_cast<float>(randomPointOnRightFace.z())
         };

         const boost::optional<QVector3D> intersectionPoint =
            DoesRayIntersectPlane(ray, rightFacePoint, POSITIVE_X_NORMAL);

         if (!intersectionPoint)
         {
            return boost::none;
         }

         if (blockOrigin.z()                 > intersectionPoint->z() &&
             blockOrigin.z() - blockDepth    < intersectionPoint->z() &&
             blockOrigin.y()                 < intersectionPoint->y() &&
             blockOrigin.y() + blockHeight   > intersectionPoint->y())
         {
            allIntersections.emplace_back(*intersectionPoint);
         }
      }

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

const double VisualizationModel::PADDING_RATIO = 0.9;
const double VisualizationModel::MAX_PADDING = 0.75;

const float VisualizationModel::BLOCK_HEIGHT = 2.0f;
const float VisualizationModel::ROOT_BLOCK_WIDTH = 1000.0f;
const float VisualizationModel::ROOT_BLOCK_DEPTH = 1000.0f;

VisualizationModel::VisualizationModel(const VisualizationParameters& parameters) :
   m_vizParameters(parameters)
{
}

void VisualizationModel::UpdateBoundingBoxes()
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
         if (currentChild->GetData().boundingBox.GetHeight() > tallestDescendant)
         {
            tallestDescendant = currentChild->GetData().boundingBox.GetHeight();
         }

         currentChild = currentChild->GetNextSibling();
      }

      node->boundingBox = Block
      {
         node->block.GetOrigin(),
         node->block.GetWidth(),
         node->block.GetHeight() + tallestDescendant,
         node->block.GetDepth()
      };
   });
}

TreeNode<VizNode>* VisualizationModel::FindNearestIntersection(
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

      const auto& closest = std::min_element(std::begin(allIntersections), std::end(allIntersections),
         [&ray] (const IntersectionPointAndNode& lhs, const IntersectionPointAndNode& rhs) noexcept
      {
         return (ray.origin().distanceToPoint(lhs.first) < ray.origin().distanceToPoint(rhs.first));
      });

      nearestIntersection = closest->second;
   }, "Node selected in ");

   return nearestIntersection;
}

Tree<VizNode>& VisualizationModel::GetTree()
{
   assert(m_theTree);
   return *m_theTree;
}

void VisualizationModel::SortNodes(Tree<VizNode>& tree)
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
