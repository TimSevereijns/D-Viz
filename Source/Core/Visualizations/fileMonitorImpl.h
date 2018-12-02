#ifndef FILEMONITORBASE_H
#define FILEMONITORBASE_H

#include <experimental/filesystem>
#include <functional>

#include "fileChangeNotification.hpp"

class FileMonitorImpl
{
public:

   FileMonitorImpl() = default;
   virtual ~FileMonitorImpl() = default;

   FileMonitorImpl(FileMonitorImpl& other) = delete;
   FileMonitorImpl& operator=(FileMonitorImpl& rhs) = delete;

   FileMonitorImpl(FileMonitorImpl&& other) = default;
   FileMonitorImpl& operator=(FileMonitorImpl&& rhs) = default;

   virtual void Start(
      const std::experimental::filesystem::path& path,
      const std::function<void (FileChangeNotification&&)>& onNotificationCallback) = 0;

   virtual void Stop() = 0;

   virtual bool IsActive() const = 0;
};

#endif // FILEMONITORBASE_H
