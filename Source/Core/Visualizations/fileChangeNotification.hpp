#ifndef FILESTATUSCHANGE_HPP
#define FILESTATUSCHANGE_HPP

#include <chrono>
#include <experimental/filesystem>
#include <functional>

#include <Tree/Tree.hpp>

enum class FileModification
{
   NONE,
   CREATED,
   DELETED,
   TOUCHED,
   RENAMED
};

struct VizBlock;

namespace Detail
{
   template<typename DataType>
   inline void HashCombine(
      std::size_t& seed,
      const DataType& value)
   {
      std::hash<DataType> hasher;
      seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
   }
}

struct FileChangeNotification
{
   FileChangeNotification() = default;

   FileChangeNotification(
      std::experimental::filesystem::path path,
      FileModification status,
      const std::chrono::high_resolution_clock::time_point& timestamp)
      :
      relativePath{ std::move(path) },
      status{ status },
      timestamp{ timestamp }
   {
   }

   // The relative path from the root of the visualization to the node that changed.
   std::experimental::filesystem::path relativePath;

   // The type of change that occurred.
   FileModification status{ FileModification::NONE };

   // A pointer to the corresponding node in the tree, should it exist.
   const typename Tree<VizBlock>::Node* node{ nullptr };

   // The time at which the change notification was produced.
   std::chrono::high_resolution_clock::time_point timestamp;

   friend bool operator==(
      const FileChangeNotification& lhs,
      const FileChangeNotification& rhs)
   {
      return lhs.node == rhs.node
         && lhs.relativePath == rhs.relativePath
         && lhs.status == rhs.status;
   }
};

namespace std
{
   template<>
   struct less<FileChangeNotification>
   {
      /**
       * @returns True if the left-hand side argument is less than the right-hand side argument.
       */
      bool operator()(
         const FileChangeNotification& lhs,
         const FileChangeNotification& rhs) const
      {
         return lhs.timestamp < rhs.timestamp;
      }
   };

   template <>
   struct hash<FileChangeNotification>
   {
      /**
       * @returns A hash based on the path of the changed file, and the type of change that
       * occurred.
       */
      std::size_t operator()(const FileChangeNotification& notification) const
      {
         std::hash<std::wstring> hasher;

         auto hash = hasher(notification.relativePath.wstring());
         Detail::HashCombine(hash, static_cast<int>(notification.status));

         return hash;
      }
   };
}

#endif // FILESTATUSCHANGE_HPP
