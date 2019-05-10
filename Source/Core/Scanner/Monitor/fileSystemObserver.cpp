#include "Scanner/Monitor/fileSystemObserver.h"
#include "Utilities/utilities.hpp"
#include "constants.h"

#include <gsl/gsl_assert>
#include <spdlog/spdlog.h>

namespace
{
    /**
     * @brief Logs File System changes.
     */
    void LogFileSystemEvent(const FileEvent& event)
    {
        switch (event.eventType) {
            case FileEventType::CREATED:
                spdlog::get(Constants::Logging::FILESYSTEM_LOG)
                    ->info(fmt::format("Create: {}", event.path.string()));
                break;

            case FileEventType::DELETED:
                spdlog::get(Constants::Logging::FILESYSTEM_LOG)
                    ->info(fmt::format("Deleted: {}", event.path.string()));
                break;

            case FileEventType::TOUCHED:
                spdlog::get(Constants::Logging::FILESYSTEM_LOG)
                    ->info(fmt::format("Modified: {}", event.path.string()));
                break;

            case FileEventType::RENAMED:
                spdlog::get(Constants::Logging::FILESYSTEM_LOG)
                    ->info(fmt::format("Renamed: {}", event.path.string()));
                break;

            default:
                std::abort();
        }
    }
} // namespace

FileSystemObserver::FileSystemObserver(
    std::unique_ptr<FileMonitorBase> fileMonitor, std::filesystem::path path)
    : m_fileSystemMonitor{ std::move(fileMonitor) }, m_rootPath{ std::move(path) }
{
}

FileSystemObserver::~FileSystemObserver()
{
    StopMonitoring();

    if (m_fileSystemNotificationProcessor.joinable()) {
        m_fileSystemNotificationProcessor.join();
    }
}

void FileSystemObserver::StartMonitoring(Tree<VizBlock>::Node* rootNode)
{
    Expects(rootNode != nullptr);
    m_rootNode = rootNode;

    if (m_rootPath.empty() || !std::filesystem::exists(m_rootPath)) {
        return;
    }

    auto callback = [&](FileEvent && event) noexcept
    {
        m_fileEvents.Emplace(std::move(event));
    };

    m_fileSystemMonitor->Start(m_rootPath, std::move(callback));
    m_fileSystemNotificationProcessor = std::thread{ [&] { ProcessChanges(); } };
}

void FileSystemObserver::StopMonitoring()
{
    if (!m_fileSystemMonitor->IsActive()) {
        return;
    }

    m_fileSystemMonitor->Stop();
    m_shouldKeepProcessingNotifications.store(false);
    m_fileEvents.AbandonWait();
}

void FileSystemObserver::WaitForNextChange()
{
    // @todo Use model updates queue instead!
    std::unique_lock<std::mutex> lock{ m_eventNotificationMutex };
    m_eventNotificationReady.wait(lock, [&]() { return !m_pendingModelUpdates.empty(); });
}

void FileSystemObserver::ProcessChanges()
{
    while (m_shouldKeepProcessingNotifications) {
        const auto event = m_fileEvents.WaitAndPop();
        if (!event) {
            // If we got here, it may indicates that the wait operation has probably been abandoned
            // due to a DTOR invocation.
            continue;
        }

        LogFileSystemEvent(*event);

        //        const auto successfullyAssociated = AssociateNotificationWithNode(*notification);
        //        if (!successfullyAssociated) {
        //            return;
        //        }

        // @todo Should there be an upper limit on the number of changes that can be in the
        // queue at any given time?
        m_pendingVisualUpdates.Emplace(*event);
        m_pendingModelUpdates.emplace(
            std::move(event->path), *event); //< @todo Is this even necessary?

        std::lock_guard<std::mutex> guard{ m_eventNotificationMutex };
        m_eventNotificationReady.notify_one();
    }
}

bool FileSystemObserver::IsActive() const
{
    return m_fileSystemMonitor->IsActive();
}

boost::optional<FileEvent> FileSystemObserver::FetchNextChange()
{
    FileEvent notification;

    const auto retrievedNotification = m_pendingVisualUpdates.TryPop(notification);
    if (!retrievedNotification) {
        return boost::none;
    }

    return std::move(notification);
}
