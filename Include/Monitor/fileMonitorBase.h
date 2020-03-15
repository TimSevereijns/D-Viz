#ifndef FILEMONITORBASE_H
#define FILEMONITORBASE_H

#include <filesystem>
#include <functional>

#include "fileChangeNotification.hpp"

class FileMonitorBase
{
  public:
    FileMonitorBase() = default;
    virtual ~FileMonitorBase() = default;

    FileMonitorBase(FileMonitorBase& other) = delete;
    FileMonitorBase& operator=(FileMonitorBase& rhs) = delete;

    FileMonitorBase(FileMonitorBase&& other) = default;
    FileMonitorBase& operator=(FileMonitorBase&& rhs) = default;

    virtual void Start(
        const std::filesystem::path& path,
        std::function<void(FileEvent&&)> onNotificationCallback) = 0;

    virtual void Stop() = 0;

    virtual bool IsActive() const = 0;
};

#endif // FILEMONITORBASE_H