#ifndef FILESTATUSCHANGE_HPP
#define FILESTATUSCHANGE_HPP

#include <experimental/filesystem>

enum class FileSystemChange
{
   CREATED,
   DELETED,
   MODIFIED,
   RENAMED
};

struct FileChangeNotification
{
   std::experimental::filesystem::path path;
   FileSystemChange status;
};

#endif // FILESTATUSCHANGE_HPP
