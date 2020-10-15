#ifndef FILEMONITORBASE_H
#define FILEMONITORBASE_H

#include <filesystem>
#include <functional>

#include "fileChangeNotification.hpp"

/**
 * @brief Abstract base class for file monitoring classes.
 */
class FileMonitorBase
{
  public:
    FileMonitorBase() = default;
    virtual ~FileMonitorBase() noexcept = default;

    FileMonitorBase(FileMonitorBase& other) = delete;
    FileMonitorBase& operator=(FileMonitorBase& rhs) = delete;

    FileMonitorBase(FileMonitorBase&& other) = default;
    FileMonitorBase& operator=(FileMonitorBase&& rhs) = default;

    /**
     * @brief Starts file system monitoring.
     *
     * @param[in] callback          Callback to be invoked when a filesystem event occurs.
     */
    virtual void Start(
        const std::filesystem::path& path,
        std::function<void(FileEvent&&)> onNotificationCallback) = 0;

    /**
     * @brief Stop file system monitoring.
     */
    virtual void Stop() = 0;

    /**
     * @returns True if the file system observer is actively monitoring; false othwerise.
     */
    virtual bool IsActive() const = 0;
};

#endif // FILEMONITORBASE_H
