#ifndef WINDOWSFILEMONITOR_H
#define WINDOWSFILEMONITOR_H

#include <QtGlobal>

#ifdef Q_OS_WIN

#include "Utilities/threadSafeQueue.hpp"
#include "fileChangeNotification.hpp"
#include "fileMonitorBase.h"

#include <array>
#include <filesystem>
#include <functional>
#include <optional>
#include <thread>
#include <vector>

#include <Windows.h>

#include <WinBase.h>
#include <fileapi.h>

namespace Detail
{
    /**
     * @brief Wrapper around the two event handles that we care when monitoring the filesystem.
     */
    class FileMonitorEventHandles
    {
      public:
        ~FileMonitorEventHandles()
        {
            for (auto& handle : m_handles) {
                if (handle != nullptr && handle != INVALID_HANDLE_VALUE) {
                    CloseHandle(handle);
                    handle = nullptr;
                }
            }
        }

        void SetExitHandle(HANDLE handle) noexcept
        {
            m_handles[0] = handle;
        }

        void SetNotificationHandle(HANDLE handle) noexcept
        {
            m_handles[1] = handle;
        }

        HANDLE GetExitHandle() const noexcept
        {
            return m_handles[0];
        }

        HANDLE GetNotificationHandle() const noexcept
        {
            return m_handles[1];
        }

        const HANDLE* Data() const noexcept
        {
            return m_handles.data();
        }

        constexpr auto Size() const noexcept
        {
            return static_cast<DWORD>(m_handles.size());
        }

      private:
        std::array<HANDLE, 2> m_handles{ INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE }; // NOLINT
    };
} // namespace Detail

/**
 * @brief The WindowsFileMonitor class
 */
class WindowsFileMonitor : public FileMonitorBase
{
  public:
    WindowsFileMonitor() = default;

    ~WindowsFileMonitor() noexcept override;

    WindowsFileMonitor(const WindowsFileMonitor& other) = delete;
    WindowsFileMonitor& operator=(const WindowsFileMonitor& other) = delete;

    WindowsFileMonitor(WindowsFileMonitor&& other) = default;
    WindowsFileMonitor& operator=(WindowsFileMonitor&& other) = default;

    /**
     * @brief Starts monitoring the file system for changes.
     *
     * @param[in] path            The root directory to watch.
     */
    void Start(
        const std::filesystem::path& path,
        const std::function<void(FileEvent&&)>& onNotificationCallback) override;

    /**
     * @brief Stops monitoring the file system for changes.
     */
    void Stop() override;

    /**
     * @returns True if the file system monitor is actively monitoring.
     */
    bool IsActive() const override;

  private:
    void Monitor();
    void ShutdownThread();

    void AwaitNotification();
    void RetrieveNotification();
    void ProcessNotification();

    std::atomic_bool m_isActive{ false };
    std::atomic_bool m_keepMonitoring{ true };

    HANDLE m_fileHandle{ INVALID_HANDLE_VALUE };

    Detail::FileMonitorEventHandles m_events;

    OVERLAPPED m_ioBuffer;

    std::vector<std::byte> m_notificationBuffer;

    std::thread m_monitoringThread;

    std::function<void(FileEvent&&)> m_notificationCallback;

    std::optional<std::wstring> m_pendingRenameEvent;

    std::filesystem::path m_pathBeingMonitored;
};

#endif // Q_OS_WIN

#endif // WINDOWSFILEMONITOR_H
