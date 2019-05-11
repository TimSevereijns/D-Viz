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
}

void FileSystemObserver::StartMonitoring(const std::function<void(FileEvent&&)>& callback)
{
    if (m_rootPath.empty() || !std::filesystem::exists(m_rootPath)) {
        return;
    }

    m_fileSystemMonitor->Start(m_rootPath, callback);
}

void FileSystemObserver::StopMonitoring() noexcept
{
    if (!m_fileSystemMonitor->IsActive()) {
        return;
    }

    m_fileSystemMonitor->Stop();
}

bool FileSystemObserver::IsActive() const
{
    return m_fileSystemMonitor->IsActive();
}
