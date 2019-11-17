#include "Scanner/Monitor/fileSystemObserver.h"
#include "Utilities/utilities.hpp"
#include "constants.h"

#include <gsl/gsl_assert>
#include <spdlog/spdlog.h>

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

    try {
        m_fileSystemMonitor->Start(m_rootPath, callback);
    } catch (const std::runtime_error& exception) {
        const auto& log = spdlog::get(Constants::Logging::DefaultLog);
        log->error(
            "Encountered an error starting filesystem monitoring logic. Error: {}.",
            exception.what());
    }
}

void FileSystemObserver::StopMonitoring()
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
