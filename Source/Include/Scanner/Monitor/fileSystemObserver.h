#ifndef FILESYSEMOBSERVER_H
#define FILESYSEMOBSERVER_H

#include "Utilities/threadSafeQueue.hpp"
#include "Visualizations/vizBlock.h"
#include "fileChangeNotification.hpp"
#include "fileMonitorBase.h"

#include <atomic>
#include <filesystem>
#include <memory>
#include <optional>
#include <thread>

/**
 * @brief Observes the filesystem for changes.
 */
class FileSystemObserver
{
  public:
    /**
     * @brief Constructs, but does not start, a file system observer.
     *
     * @param[in] fileMonitor        Dependency injected file monitoring implementation.
     * @param[in] path               The path to the location to be recursively observed.
     */
    FileSystemObserver(std::unique_ptr<FileMonitorBase> fileMonitor, std::filesystem::path path);

    /**
     * @brief Halts monitoring and destroys the observer.
     */
    ~FileSystemObserver();

    /**
     * @brief Starts file system monitoring.
     *
     * @param[in] callback          Callback to be invoked when a filesystem event occurs.
     */
    void StartMonitoring(const std::function<void(FileEvent&&)>& callback);

    /**
     * @brief Stop file system monitoring.
     */
    void StopMonitoring();

    /**
     * @returns True if the file system observer is actively monitoring; false othwerise.
     */
    bool IsActive() const;

  private:
    std::unique_ptr<FileMonitorBase> m_fileSystemMonitor;

    std::filesystem::path m_rootPath;
};

#endif // FILESYSEMOBSERVER_H
