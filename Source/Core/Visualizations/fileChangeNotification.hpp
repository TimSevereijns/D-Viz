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

struct FileChangeNotification
{
   FileChangeNotification() = default;

   FileChangeNotification(
      std::experimental::filesystem::path path,
      FileModification status)
      :
      relativePath{ std::move(path) },
      status{ status }
   {
   }

   // The relative path from the root of the visualization to the node that changed.
   std::experimental::filesystem::path relativePath;

   // The type of change that occurred.
   FileModification status{ FileModification::NONE };

   // A pointer to the corresponding node in the tree, should it exist.
   const typename Tree<VizBlock>::Node* node{ nullptr };

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
   struct less<std::experimental::filesystem::path>
   {
      /**
       * @returns True if the left-hand side argument is less than the right-hand side argument.
       */
      bool operator()(
         const std::experimental::filesystem::path& lhs,
         const std::experimental::filesystem::path& rhs) const
      {
         return lhs.native() < rhs.native();
      }
   };

   template<>
   struct hash<std::experimental::filesystem::path>
   {
      /**
       * @returns A hash based on the path of the changed file.
       */
      std::size_t operator()(const std::experimental::filesystem::path& path) const
      {
         return std::hash<std::experimental::filesystem::path::string_type>{ }(path.native());
      }
   };
}

#endif // FILESTATUSCHANGE_HPP
