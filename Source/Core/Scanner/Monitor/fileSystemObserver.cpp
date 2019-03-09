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
    void LogFileSystemEvent(const FileChangeNotification& notification)
    {
        switch (notification.status) {
            case FileModification::CREATED:
                spdlog::get(Constants::Logging::FILESYSTEM_LOG)
                    ->info(fmt::format("Create: {}", notification.relativePath.string()));
                break;

            case FileModification::DELETED:
                spdlog::get(Constants::Logging::FILESYSTEM_LOG)
                    ->info(fmt::format("Deleted: {}", notification.relativePath.string()));
                break;

            case FileModification::TOUCHED:
                spdlog::get(Constants::Logging::FILESYSTEM_LOG)
                    ->info(fmt::format("Modified: {}", notification.relativePath.string()));
                break;

            case FileModification::RENAMED:
                spdlog::get(Constants::Logging::FILESYSTEM_LOG)
                    ->info(fmt::format("Renamed: {}", notification.relativePath.string()));
                break;

            default:
                std::abort();
        }
    }
} // namespace

FileSystemObserver::FileSystemObserver(
    std::unique_ptr<FileMonitorBase> fileMonitor, std::experimental::filesystem::path path)
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

    if (m_rootPath.empty() || !std::experimental::filesystem::exists(m_rootPath)) {
        return;
    }

    auto callback = [&](FileChangeNotification && notification) noexcept
    {
        m_fileChangeNotifications.Emplace(std::move(notification));
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
    m_fileChangeNotifications.AbandonWait();
}

bool FileSystemObserver::AssociateNotificationWithNode(FileChangeNotification& notification)
{
    Expects(notification.node == nullptr);

    auto* node = Utilities::FindNodeUsingRelativePath(m_rootNode, notification.relativePath);
    notification.node = node;

    return node != nullptr;
}

void FileSystemObserver::ProcessChanges()
{
    while (m_shouldKeepProcessingNotifications) {
        const auto notification = m_fileChangeNotifications.WaitAndPop();
        if (!notification) {
            // If we got here, it may indicates that the wait operation has probably been abandoned
            // due to a DTOR invocation.
            continue;
        }

        LogFileSystemEvent(*notification);

        const auto successfullyAssociated = AssociateNotificationWithNode(*notification);
        if (successfullyAssociated) {
            // @todo Should there be an upper limit on the number of changes that can be in the
            // queue at any given time?
            m_pendingVisualUpdates.Emplace(*notification);

            auto absolutePath =
                std::experimental::filesystem::absolute(notification->relativePath, m_rootPath);

            m_pendingModelUpdates.emplace(std::move(absolutePath), *notification);
        }
    }
}

bool FileSystemObserver::IsActive() const
{
    return m_fileSystemMonitor->IsActive();
}

boost::optional<FileChangeNotification> FileSystemObserver::FetchNextChange()
{
    FileChangeNotification notification;

    const auto retrievedNotification = m_pendingVisualUpdates.TryPop(notification);
    if (!retrievedNotification) {
        return boost::none;
    }

    return std::move(notification);
}
