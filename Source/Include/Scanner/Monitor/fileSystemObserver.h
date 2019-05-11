#ifndef FILESYSEMOBSERVER_H
#define FILESYSEMOBSERVER_H

#include "Utilities/threadSafeQueue.hpp"
#include "Visualizations/vizBlock.h"
#include "fileChangeNotification.hpp"
#include "fileMonitorBase.h"

#include <Tree/Tree.hpp>

#include <atomic>
#include <filesystem>
#include <memory>
#include <optional>
#include <thread>
#include <unordered_map>

/**
 * @brief Recursively observes the file system for changes to any files.
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
     * @param[in] rootNode           The root node of the existing file system model against which
     *                               any file system changes will be compared.
     */
    void StartMonitoring(Tree<VizBlock>::Node* rootNode);

    /**
     * @brief Stop file system monitoring.
     */
    void StopMonitoring();

    /**
     * @returns True if the file system observer is actively monitoring; false othwerise.
     */
    bool IsActive() const;

    /**
     * @brief Fetches the next pending file system change.
     *
     * @returns A notification is one is available, and boost::none if nothing is available.
     */
    std::optional<FileEvent> FetchNextChange();

    void WaitForNextChange();

  private:
    bool AssociateNotificationWithNode(FileEvent& notification);

    void ProcessChanges();

    std::atomic_bool m_shouldKeepProcessingNotifications{ true };

    std::unique_ptr<FileMonitorBase> m_fileSystemMonitor;

    // This queue contains raw notifications of file system changes that still need to be
    // parsed and the turned into tree node change notifications.
    ThreadSafeQueue<FileEvent> m_fileEvents;

    // This queue contains pending tree node change notifications. These notifications
    // still need to be retrieved by the view so that the UI can be updated to reflect filesystem
    // activity.
    ThreadSafeQueue<FileEvent> m_pendingVisualUpdates;

    // This map tracks changes that will need to be applied to the treemap once the user refreshes
    // the visualization to reflect filesystem changes.
    std::unordered_map<std::filesystem::path, FileEvent> m_pendingModelUpdates;

    std::thread m_fileSystemNotificationProcessor;

    std::condition_variable m_eventNotificationReady;
    std::mutex m_eventNotificationMutex;

    std::filesystem::path m_rootPath;

    Tree<VizBlock>::Node* m_rootNode{ nullptr };
};

#endif // FILESYSEMOBSERVER_H
