#include "visualization.h"

#include "../constants.h"
#include "../DriveScanner/driveScanningUtilities.h"
#include "fileChangeNotification.hpp"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/optional.hpp>

#include <spdlog/spdlog.h>
#include <Stopwatch/Stopwatch.hpp>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>

#include <QColor>
#include <QRectF>

// @todo Replace the use of this class with something more stable.
#include <Qt3DRender/private/qray3d_p.h>

namespace
{
   constexpr QVector3D POSITIVE_X_NORMAL{  1.0f,  0.0f,  0.0f };
   constexpr QVector3D POSITIVE_Y_NORMAL{  0.0f,  1.0f,  0.0f };
   constexpr QVector3D POSITIVE_Z_NORMAL{  0.0f,  0.0f,  1.0f };
   constexpr QVector3D NEGATIVE_X_NORMAL{ -1.0f,  0.0f,  0.0f };
   constexpr QVector3D NEGATIVE_Y_NORMAL{  0.0f, -1.0f,  0.0f };
   constexpr QVector3D NEGATIVE_Z_NORMAL{  0.0f,  0.0f, -1.0f };

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
      const Qt3DRender::RayCasting::QRay3D& ray,
      const QVector3D& pointOnPlane,
      const QVector3D& planeNormal)
   {
      constexpr auto epsilon{ 0.0001 };

      const double denominator = QVector3D::dotProduct(ray.direction(), planeNormal);
      if (std::abs(denominator) < epsilon)
      {
         return boost::none;
      }

      const double numerator = QVector3D::dotProduct(pointOnPlane - ray.origin(), planeNormal);

      const double scalar = numerator / denominator;
      const bool doesRayHitPlane = std::abs(scalar) > epsilon;

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
      const Qt3DRender::RayCasting::QRay3D& ray,
      const std::vector<QVector3D>& allIntersections)
   {
      const auto& closest = std::min_element(
         std::begin(allIntersections),
         std::end(allIntersections),
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
      const Qt3DRender::RayCasting::QRay3D& ray,
      const Block& block)
   {
      std::vector<QVector3D> allIntersections;

      const auto blockOrigin = block.GetOrigin();
      const auto blockHeight = block.GetHeight();
      const auto blockWidth = block.GetWidth();
      const auto blockDepth = block.GetDepth();

      { // Perform hit detection on the top face:
         const auto randomPointOnTopFace = blockOrigin + PrecisePoint{ 0, blockHeight, 0 };
         const QVector3D pointOnPlane
         {
            static_cast<float>(randomPointOnTopFace.x()),
            static_cast<float>(randomPointOnTopFace.y()),
            static_cast<float>(randomPointOnTopFace.z())
         };

         const boost::optional<QVector3D> intersectionPoint =
            DoesRayIntersectPlane(ray, pointOnPlane, POSITIVE_Y_NORMAL);

         if (intersectionPoint &&
             blockOrigin.x()                 < intersectionPoint->x() &&
             blockOrigin.x() + blockWidth    > intersectionPoint->x() &&
             blockOrigin.z()                 > intersectionPoint->z() &&
             blockOrigin.z() - blockDepth    < intersectionPoint->z())
         {
            allIntersections.emplace_back(*intersectionPoint);
         }
      }

      { // Perform hit detection on the front face:
         const auto randomPointOnFrontFace = blockOrigin;
         const QVector3D pointOnPlane
         {
            static_cast<float>(randomPointOnFrontFace.x()),
            static_cast<float>(randomPointOnFrontFace.y()),
            static_cast<float>(randomPointOnFrontFace.z())
         };

         const boost::optional<QVector3D> intersectionPoint =
            DoesRayIntersectPlane(ray, pointOnPlane, POSITIVE_Z_NORMAL);

         if (intersectionPoint &&
             blockOrigin.x()                 < intersectionPoint->x() &&
             blockOrigin.x() + blockWidth    > intersectionPoint->x() &&
             blockOrigin.y()                 < intersectionPoint->y() &&
             blockOrigin.y() + blockHeight   > intersectionPoint->y())
         {
            allIntersections.emplace_back(*intersectionPoint);
         }
      }

      { // Perform hit detection on the back face:
         const auto randomPointOnBackFace = blockOrigin + PrecisePoint{ 0, 0, -blockDepth };
         const QVector3D pointOnPlane
         {
            static_cast<float>(randomPointOnBackFace.x()),
            static_cast<float>(randomPointOnBackFace.y()),
            static_cast<float>(randomPointOnBackFace.z())
         };

         const boost::optional<QVector3D> intersectionPoint =
            DoesRayIntersectPlane(ray, pointOnPlane, NEGATIVE_Z_NORMAL);

         if (intersectionPoint &&
             blockOrigin.x()                 < intersectionPoint->x() &&
             blockOrigin.x() + blockWidth    > intersectionPoint->x() &&
             blockOrigin.y()                 < intersectionPoint->y() &&
             blockOrigin.y() + blockHeight   > intersectionPoint->y())
         {
            allIntersections.emplace_back(*intersectionPoint);
         }
      }

      { // Perform hit detection on the left face:
         const auto randomPointOnLeftFace = blockOrigin;
         const QVector3D pointOnPlane
         {
            static_cast<float>(randomPointOnLeftFace.x()),
            static_cast<float>(randomPointOnLeftFace.y()),
            static_cast<float>(randomPointOnLeftFace.z())
         };

         const boost::optional<QVector3D> intersectionPoint =
            DoesRayIntersectPlane(ray, pointOnPlane, NEGATIVE_X_NORMAL);

         if (intersectionPoint &&
             blockOrigin.z()                 > intersectionPoint->z() &&
             blockOrigin.z() - blockDepth    < intersectionPoint->z() &&
             blockOrigin.y()                 < intersectionPoint->y() &&
             blockOrigin.y() + blockHeight   > intersectionPoint->y())
         {
            allIntersections.emplace_back(*intersectionPoint);
         }
      }

      { // Perform hit detection on the right face:
         const auto randomPointOnRightFace = blockOrigin + PrecisePoint{ blockWidth, 0, 0 };
         const QVector3D pointOnPlane
         {
            static_cast<float>(randomPointOnRightFace.x()),
            static_cast<float>(randomPointOnRightFace.y()),
            static_cast<float>(randomPointOnRightFace.z())
         };

         const boost::optional<QVector3D> intersectionPoint =
            DoesRayIntersectPlane(ray, pointOnPlane, POSITIVE_X_NORMAL);

         if (intersectionPoint &&
             blockOrigin.z()                 > intersectionPoint->z() &&
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
   void AdvanceToNextNonDescendant(Tree<VizBlock>::Node*& node)
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

   struct IntersectionInfo
   {
      QVector3D point;
      Tree<VizBlock>::Node* node;
   };

   /**
    * @brief Iterates over all nodes in the scene, placing all intersections in a vector.
    *
    * @param[in] ray                The ray to be shot into the scene.
    * @param[in] camera             The camera from which the ray is shot.
    * @param[in] parameters         Additional visualization parameters.
    * @param[in] node               The current node being hit-tested.
    */
   std::vector<IntersectionInfo> FindAllIntersections(
      const Qt3DRender::RayCasting::QRay3D& ray,
      const Camera& camera,
      const Settings::VisualizationParameters& parameters,
      Tree<VizBlock>::Node* node)
   {
      std::vector<IntersectionInfo> allIntersections;

      assert(node);
      while (node)
      {
         if (node->GetData().file.size < parameters.minimumFileSize ||
            (parameters.onlyShowDirectories && node->GetData().file.type != FileType::DIRECTORY))
         {
            AdvanceToNextNonDescendant(node);

            continue;
         }

         const auto& boundingBoxIntersection =
            DoesRayIntersectBlock(ray, node->GetData().boundingBox);

         if (boundingBoxIntersection)
         {
            const auto& blockIntersection = DoesRayIntersectBlock(ray, node->GetData().block);
            if (blockIntersection && camera.IsPointInFrontOfCamera(*blockIntersection))
            {
               allIntersections.emplace_back(IntersectionInfo{ *blockIntersection, node });
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

   /**
    * @brief LogFileSystemEvent
    *
    * @param notification
    */
   void LogFileSystemEvent(const FileChangeNotification& notification)
   {
      switch (notification.status)
      {
         case FileSystemChange::CREATED:
            spdlog::get(Constants::Logging::FILESYSTEM_LOG)->info(
               fmt::format("Create: {}", notification.relativePath.string()));
            break;

         case FileSystemChange::DELETED:
            spdlog::get(Constants::Logging::FILESYSTEM_LOG)->info(
               fmt::format("Deleted: {}", notification.relativePath.string()));
            break;

         case FileSystemChange::MODIFIED:
            spdlog::get(Constants::Logging::FILESYSTEM_LOG)->info(
               fmt::format("Modified: {}", notification.relativePath.string()));
            break;

         case FileSystemChange::RENAMED:
            spdlog::get(Constants::Logging::FILESYSTEM_LOG)->info(
               fmt::format("Renamed: {}", notification.relativePath.string()));
            break;

         default:
            assert(false);
      }
   }
}

const double VisualizationModel::PADDING_RATIO{ 0.9 };
const double VisualizationModel::MAX_PADDING{ 0.75 };

const float VisualizationModel::BLOCK_HEIGHT{ 2.0f };
const float VisualizationModel::ROOT_BLOCK_WIDTH{ 1000.0f };
const float VisualizationModel::ROOT_BLOCK_DEPTH{ 1000.0f };

VisualizationModel::VisualizationModel(const std::experimental::filesystem::path& path) :
   m_rootPath{ path }
{
}

VisualizationModel::~VisualizationModel() noexcept
{
   StopMonitoringFileSystem();

   if (m_fileSystemNotificationProcessor.joinable())
   {
      m_fileSystemNotificationProcessor.join();
   }
}

void VisualizationModel::UpdateBoundingBoxes()
{
   assert(m_hasDataBeenParsed);
   assert(m_fileTree);

   if (!m_hasDataBeenParsed)
   {
      return;
   }

   std::for_each(std::begin(*m_fileTree), std::end(*m_fileTree),
      [] (auto& node) noexcept
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

Tree<VizBlock>::Node* VisualizationModel::FindNearestIntersection(
   const Camera& camera,
   const Qt3DRender::RayCasting::QRay3D& ray,
   const Settings::VisualizationParameters& parameters) const
{
   if (!m_hasDataBeenParsed)
   {
      return nullptr;
   }

   Tree<VizBlock>::Node* nearestIntersection = nullptr;

   Stopwatch<std::chrono::microseconds>([&] () noexcept
   {
      const auto root = m_fileTree->GetRoot();
      const auto intersections = FindAllIntersections(ray, camera, parameters, root);

      if (intersections.empty())
      {
         return;
      }

      const auto closest = std::min_element(std::begin(intersections), std::end(intersections),
         [&ray] (const auto& lhs, const auto& rhs) noexcept
      {
         return (ray.origin().distanceToPoint(lhs.point) < ray.origin().distanceToPoint(rhs.point));
      });

      nearestIntersection = closest->node;
   },
   [] (const auto& elapsed, const auto& units) noexcept
   {
      spdlog::get(Constants::Logging::DEFAULT_LOG)->info(
         fmt::format("Selected node in: {} {}", elapsed.count(), units));
   });

   return nearestIntersection;
}

Tree<VizBlock>& VisualizationModel::GetTree()
{
   assert(m_fileTree);
   return *m_fileTree;
}

const Tree<VizBlock>& VisualizationModel::GetTree() const
{
   assert(m_fileTree);
   return *m_fileTree;
}

const std::vector<const Tree<VizBlock>::Node*>& VisualizationModel::GetHighlightedNodes() const
{
   return m_highlightedNodes;
}

std::vector<const Tree<VizBlock>::Node*>& VisualizationModel::GetHighlightedNodes()
{
   return m_highlightedNodes;
}

void VisualizationModel::ClearHighlightedNodes()
{
   if (m_highlightedNodes.size() == 0)
   {
      return;
   }

   m_highlightedNodes.clear();
}

void VisualizationModel::SelectNode(const Tree<VizBlock>::Node& node)
{
   m_selectedNode = &node;
}

const Tree<VizBlock>::Node* VisualizationModel::GetSelectedNode()
{
   return m_selectedNode;
}

void VisualizationModel::ClearSelectedNode()
{
   m_selectedNode = nullptr;
}

TreemapMetadata VisualizationModel::GetTreemapMetadata()
{
   return m_metadata;
}

void VisualizationModel::SetTreemapMetadata(TreemapMetadata&& data)
{
   m_metadata = data;
}

void VisualizationModel::HighlightAncestors(const Tree<VizBlock>::Node& node)
{
   auto* currentNode = node.GetParent();
   while (currentNode)
   {
      m_highlightedNodes.emplace_back(currentNode);
      currentNode = currentNode->GetParent();
   }
}

void VisualizationModel::HighlightDescendants(
   const Tree<VizBlock>::Node& node,
   const Settings::VisualizationParameters& parameters)
{
   std::for_each(
      Tree<VizBlock>::LeafIterator{ &node },
      Tree<VizBlock>::LeafIterator{ },
      [&] (const auto& node)
   {
      if ((parameters.onlyShowDirectories && node->file.type != FileType::DIRECTORY)
         || node->file.size < parameters.minimumFileSize)
      {
         return;
      }

      m_highlightedNodes.emplace_back(&node);
   });
}

void VisualizationModel::HighlightMatchingFileExtension(
   const Tree<VizBlock>::Node& sampleNode,
   const Settings::VisualizationParameters& parameters)
{
   std::for_each(
      Tree<VizBlock>::LeafIterator{ GetTree().GetRoot() },
      Tree<VizBlock>::LeafIterator{ },
      [&] (const auto& node)
   {
      if ((parameters.onlyShowDirectories && node->file.type != FileType::DIRECTORY)
         || node->file.size < parameters.minimumFileSize
         || node->file.extension != sampleNode->file.extension)
      {
         return;
      }

      m_highlightedNodes.emplace_back(&node);
   });
}

void VisualizationModel::HighlightMatchingFileName(
   const std::wstring& searchQuery,
   const Settings::VisualizationParameters& parameters,
   bool shouldSearchFiles,
   bool shouldSearchDirectories)
{
   std::wstring fileAndExtension;
   fileAndExtension.resize(260); ///< Resize to prevent reallocation with append operations.

   const auto lowercaseQuery = boost::algorithm::to_lower_copy(searchQuery);

   std::for_each(
      Tree<VizBlock>::PostOrderIterator{ GetTree().GetRoot() },
      Tree<VizBlock>::PostOrderIterator{ },
      [&] (const auto& node)
   {
      const auto& file = node->file;

      if (file.size < parameters.minimumFileSize
         || (!shouldSearchDirectories && file.type == FileType::DIRECTORY)
         || (!shouldSearchFiles && file.type == FileType::REGULAR))
      {
         return;
      }

      fileAndExtension = file.name;
      fileAndExtension.append(file.extension);

      boost::algorithm::to_lower(fileAndExtension);

      // @note We're converting everything to lowercase beforehand
      // (instead of using `boost::icontains(...)`), since doing so is significantly faster.
      if (!boost::contains(fileAndExtension, lowercaseQuery))
      {
         return;
      }

      m_highlightedNodes.emplace_back(&node);
   });
}

void VisualizationModel::StartMonitoringFileSystem()
{
   if (m_rootPath.empty() || !std::experimental::filesystem::exists(m_rootPath))
   {
      assert(false);
      return;
   }

   auto callback = [&] (FileChangeNotification&& notification) noexcept
   {
      m_fileChangeNotifications.Emplace(std::move(notification));
   };

   m_fileSystemMonitor.Start(m_rootPath, std::move(callback));
   m_fileSystemNotificationProcessor = std::thread{ [&] { ProcessFileSystemChanges(); } };
}

void VisualizationModel::StopMonitoringFileSystem()
{
   if (!IsFileSystemBeingMonitored())
   {
      return;
   }

   m_fileSystemMonitor.Stop();
   m_shouldKeepProcessingNotifications.store(false);
   m_fileChangeNotifications.AbandonWait();
}

void VisualizationModel::ProcessFileSystemChanges()
{
   while (m_shouldKeepProcessingNotifications)
   {
      const auto notification = m_fileChangeNotifications.WaitAndPop();
      if (!notification)
      {
         // If we got here, it may indicates that the wait operation has probably been abandoned
         // due to a DTOR invocation.
         continue;
      }

      LogFileSystemEvent(*notification);

      const auto successfullyResolved = ResolveNotification(*notification);
      if (successfullyResolved)
      {
         m_pendingGraphicalUpdates.Emplace(*notification);
         m_pendingModelChanges.emplace(*notification);
      }
   }
}

bool VisualizationModel::ResolveNotification(FileChangeNotification& notification)
{
   auto* node = FindNodeUsingRelativePath(notification.relativePath);
   notification.node = node;

   return node != nullptr;
}

void VisualizationModel::UpdateTreemap()
{
   for (const auto& change : m_pendingModelChanges)
   {
      UpdateAffectedNodes(change);
   }

   // @todo Sort the tree.
   // @todo Update all sizes.
}

void VisualizationModel::UpdateAffectedNodes(const FileChangeNotification& notification)
{
   const auto absolutePath =
      std::experimental::filesystem::absolute(notification.relativePath, m_rootPath);

   std::error_code errorCode;

   if (notification.status != FileSystemChange::DELETED
      && !std::experimental::filesystem::exists(absolutePath)
      && !errorCode)
   {
      // @note The absence of a file may not necessarily indicate a bug, since there tend to be
      // a lot of transient files that may only exist for a fraction of a second. For example,
      // some applications tend to create temporary files when saving changes made to a file.

      spdlog::get(Constants::Logging::DEFAULT_LOG)->error(
         fmt::format("File no longer exists: {}", absolutePath.string()));

      return;
   }

   switch (notification.status)
   {
      case FileSystemChange::CREATED:
         OnFileCreation(notification);
         break;

      case FileSystemChange::DELETED:
         OnFileDeletion(notification);
         break;

      case FileSystemChange::MODIFIED:
         OnFileModification(notification);
         break;

      case FileSystemChange::RENAMED:
         OnFileNameChange(notification);
         break;

      default:
         assert(false);
   }
}

void VisualizationModel::OnFileCreation(const FileChangeNotification& notification)
{
   // @todo Find parent node from path:
   auto* parentNode = FindNodeUsingRelativePath(notification.relativePath.stem());

   const auto absolutePath =
      std::experimental::filesystem::absolute(notification.relativePath, m_rootPath);

   if (std::experimental::filesystem::is_directory(absolutePath)) //< @todo Check symlink status...
   {
      FileInfo directoryInfo
      {
         notification.relativePath.filename().wstring(),
         /* extension = */ L"",
         /* size = */ 0,
         FileType::DIRECTORY
      };

      parentNode->AppendChild(VizBlock{ std::move(directoryInfo) });
   }
   else // if (is_regular(...))
   {
      const auto fileSize = DriveScanning::Utilities::ComputeFileSize(absolutePath);

      FileInfo fileInfo
      {
         notification.relativePath.filename().stem().wstring(),
         notification.relativePath.filename().extension().wstring(),
         fileSize,
         FileType::REGULAR
      };

      parentNode->AppendChild(VizBlock{ std::move(fileInfo) });
   }
}

void VisualizationModel::OnFileDeletion(const FileChangeNotification& notification)
{
   auto* node = FindNodeUsingRelativePath(notification.relativePath);
   if (node)
   {
      node->DeleteFromTree();;
   }
}

void VisualizationModel::OnFileModification(const FileChangeNotification& notification)
{
   const auto absolutePath =
      std::experimental::filesystem::absolute(notification.relativePath, m_rootPath);

   if (std::experimental::filesystem::is_directory(absolutePath))
   {
      // @todo What does it mean for a directory to be modified?
   }
   else // if (is_regular(...))
   {
      const auto fileSize = DriveScanning::Utilities::ComputeFileSize(absolutePath);

      auto* node = FindNodeUsingRelativePath(notification.relativePath);
      if (node)
      {
         node->GetData().file.size = fileSize;;
      }      
   }
}

void VisualizationModel::OnFileNameChange(const FileChangeNotification& /*notification*/)
{
   // @todo Need to associate new file names with old file names in order to resolve rename events.
}

void VisualizationModel::UpdateAncestorSizes(Tree<VizBlock>::Node* node)
{
   while (node)
   {
      auto* parent = node->GetParent();
      if (parent)
      {
         const auto totalSize = std::accumulate(
            Tree<VizBlock>::SiblingIterator{ parent->GetFirstChild() },
            Tree<VizBlock>::SiblingIterator{ },
            std::uintmax_t{ 0 },
            [] (const auto runningTotal, const auto& node) noexcept
         {
            assert(node->file.size);
            return runningTotal + node->file.size;
         });

         assert(totalSize);
         parent->GetData().file.size = totalSize;
      }

      node = parent;
   }
}

Tree<VizBlock>::Node* VisualizationModel::FindNodeUsingRelativePath(
   const std::experimental::filesystem::path& path)
{
   auto* node = m_fileTree->GetRoot();

   auto filePathItr = std::begin(path);
   while (filePathItr != std::end(path))
   {
      auto matchingNodeItr = std::find_if(
         Tree<VizBlock>::SiblingIterator{ node->GetFirstChild() },
         Tree<VizBlock>::SiblingIterator{ },
         [&] (const auto& childNode)
      {
         const auto pathElement = filePathItr->wstring();
         const auto fileName = childNode->file.name + childNode->file.extension;

         return fileName == pathElement;
      });

      if (matchingNodeItr != Tree<VizBlock>::SiblingIterator{ })
      {
         node = std::addressof(*matchingNodeItr);
         ++filePathItr;
      }
      else
      {
         break;
      }
   }

   if (filePathItr != std::end(path))
   {
      return nullptr;
   }

   return node;
}

bool VisualizationModel::IsFileSystemBeingMonitored() const
{
   return m_fileSystemMonitor.IsActive();
}

boost::optional<FileChangeNotification> VisualizationModel::FetchNodeUpdate()
{
   FileChangeNotification notification;

   const auto retrievedNotification = m_pendingGraphicalUpdates.TryPop(notification);
   if (!retrievedNotification)
   {
      return boost::none;
   }

   return notification;
}

std::experimental::filesystem::path VisualizationModel::GetRootPath() const
{
   return m_rootPath;
}

void VisualizationModel::SortNodes(Tree<VizBlock>& tree)
{
   for (auto& node : tree)
   {
      node.SortChildren([] (const auto& lhs, const auto& rhs) noexcept
      {
         return lhs->file.size > rhs->file.size;
      });
   }
}