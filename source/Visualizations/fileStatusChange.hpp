#ifndef FILESTATUSCHANGE_HPP
#define FILESTATUSCHANGE_HPP

#include <experimental/filesystem>

enum class FileStatusChanged
{
   CREATED,
   DELETED,
   MODIFIED,
   RENAMED
};

struct FileAndChangeStatus
{
   std::experimental::filesystem::path path;
   FileStatusChanged status;
};

#endif // FILESTATUSCHANGE_HPP
