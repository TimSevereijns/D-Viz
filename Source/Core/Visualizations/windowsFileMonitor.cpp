#include "Visualizations/windowsFileMonitor.h"

#ifdef Q_OS_WIN

#include <exception>
#include <iostream>

#include <gsl/gsl_assert>
#include <spdlog/spdlog.h>

#include "DriveScanner/scopedHandle.h"
#include "constants.h"

namespace
{
    /**
     * @brief Advances a non-null pointer by a specified number of bytes.
     *
     * @param[in] ptr                The pointer value to advance.
     * @param[in] offset             The number of bytes by which to advance the pointer.
     *
     * @returns An advanced pointer if the input is not null.
     */
    template <typename DataType> DataType* AdvancePointer(DataType* ptr, std::ptrdiff_t offset)
    {
        if (ptr == nullptr) {
            return ptr;
        }

        return reinterpret_cast<DataType*>(reinterpret_cast<std::byte*>(ptr) + offset);
    }

    /**
     * @brief GetLastErrorAsString
     * @return
     */
    std::string GetLastErrorAsString()
    {
        DWORD errorMessageID = ::GetLastError();
        if (errorMessageID == 0) {
            return {};
        }

        LPSTR messageBuffer = nullptr;

        constexpr auto formattingOptions = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                           FORMAT_MESSAGE_FROM_SYSTEM |
                                           FORMAT_MESSAGE_IGNORE_INSERTS;

        const auto characterCount = FormatMessageA(
            formattingOptions, nullptr, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPSTR>(&messageBuffer), 0, nullptr);

        std::string message{ messageBuffer, characterCount };

        LocalFree(messageBuffer);

        return message;
    }

    /**
     * @brief LogLastError
     * @param message
     */
    void LogLastError(std::string_view message)
    {
        const auto lastError = GetLastErrorAsString();
        const auto& log = spdlog::get(Constants::Logging::DEFAULT_LOG);
        log->error("{} Last Error: {}.", message, lastError);
    }
} // namespace

WindowsFileMonitor::~WindowsFileMonitor() noexcept
{
    if (m_isActive) {
        Stop();
    }
}

bool WindowsFileMonitor::IsActive() const
{
    return m_isActive;
}

void WindowsFileMonitor::Start(
    const std::experimental::filesystem::path& path,
    const std::function<void(FileChangeNotification&&)>& onNotificationCallback)
{
    m_notificationCallback = onNotificationCallback;

    // When monitoring a file on a network drive, the size of the buffer cannot exceed 64 KiB.
    // To quote the documentation: "This is due to a packet size limitation with the underlying file
    // sharing protocols."
    //
    // In the C# documentation for the analogous `FileSystemWatcher`, Microsoft appears to default
    // the buffer's size to 8,192 bytes (or 8 KiB), so we'll do the same.
    //
    // Interestingly, a small buffer can also be significantly faster:
    // https://randomascii.wordpress.com/2018/04/17/making-windows-slower-part-1-file-access/

    using namespace Literals::Numeric::Binary;
    m_notificationBuffer.resize(8_KiB, std::byte{ 0 });

    m_fileHandle = CreateFileW(
        /* lpFileName = */ path.wstring().data(),
        /* dwDesiredAccess = */ FILE_LIST_DIRECTORY | STANDARD_RIGHTS_READ,
        /* dwShareMode = */ FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        /* lpSecurityAttributes = */ nullptr,
        /* dwCreationDisposition = */ OPEN_EXISTING,
        /* dwFlagsAndAttributes = */ FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        /* hTemplateFile = */ nullptr);

    if (!m_fileHandle || m_fileHandle == INVALID_HANDLE_VALUE) {
        const auto& log = spdlog::get(Constants::Logging::DEFAULT_LOG);
        log->error("Could not acquire handle to: {}.", path.string());

        return;
    }

    const auto exitThreadHandle = CreateEventW(
        /* lpEventAttributes = */ nullptr,
        /* bManualReset = */ true,
        /* bInitialState = */ false,
        /* lpName = */ L"D-VIZ_FILE_MONITOR_TERMINATE_THREAD");

    m_events.SetExitHandle(exitThreadHandle);

    const auto notificationHandle = CreateEventW(
        /* lpEventAttributes = */ nullptr,
        /* bManualReset = */ false,
        /* bInitialState = */ false,
        /* lpName = */ L"D-VIZ_FILE_MONITOR_NOTIFICATION");

    m_events.SetNotificationHandle(notificationHandle);

    ::ZeroMemory(&m_ioBuffer, sizeof(OVERLAPPED));
    m_ioBuffer.hEvent = notificationHandle;

    m_isActive = true;
    m_monitoringThread = std::thread{ [&] { Monitor(); } };
}

void WindowsFileMonitor::Stop()
{
    SetEvent(m_events.GetExitHandle());

    if (m_monitoringThread.joinable()) {
        m_monitoringThread.join();
    }

    Expects(m_isActive == false);

    if (m_fileHandle && m_fileHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_fileHandle);
    }
}

void WindowsFileMonitor::Monitor()
{
    while (m_keepMonitoring) {
        AwaitNotification();
    }

    m_isActive = false;
}

void WindowsFileMonitor::AwaitNotification()
{
    constexpr auto desiredNotifications = FILE_NOTIFY_CHANGE_FILE_NAME |
                                          FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_SIZE |
                                          FILE_NOTIFY_CHANGE_CREATION;

    const bool successfullyQueued = ReadDirectoryChangesW(
        /* hDirectory = */ m_fileHandle,
        /* lpBuffer = */ m_notificationBuffer.data(),
        /* nBufferLength = */ static_cast<DWORD>(m_notificationBuffer.size()),
        /* bWatchSubtree = */ TRUE,
        /* dwNotifyFilter = */ desiredNotifications,
        /* lpBytesReturned = */ nullptr,
        /* lpOverlapped = */ &m_ioBuffer,
        /* lpCompletionRoutine = */ nullptr);

    if (!successfullyQueued) {
        LogLastError("Encountered error queuing filesytem changes.");
    }

    const auto waitResult = WaitForMultipleObjects(
        /* nCount = */ m_events.Size(),
        /* lpHandles = */ m_events.Data(),
        /* bWaitAll = */ false,
        /* dwMilliseconds = */ INFINITE);

    switch (waitResult) {
        case WAIT_OBJECT_0: {
            m_keepMonitoring.store(false);

            CancelIo(m_fileHandle);

            while (!HasOverlappedIoCompleted(&m_ioBuffer)) {
                SleepEx(50, TRUE);
            }

            break;
        }
        case WAIT_OBJECT_0 + 1: {
            RetrieveNotification();
            break;
        }
        case WAIT_FAILED: {
            LogLastError("Encountered error waiting on event.");
            break;
        }
        default: {
            Expects(false);
        }
    }
}

void WindowsFileMonitor::RetrieveNotification()
{
    DWORD bytesTransferred{ 0 };

    const bool successfullyRead = GetOverlappedResult(
        /* hFile = */ m_fileHandle,
        /* lpOverlapped = */ &m_ioBuffer,
        /* lpNumberOfBytesTransferred = */ &bytesTransferred,
        /* bWait = */ false);

    if (successfullyRead && bytesTransferred > 0) {
        ProcessNotification();
    } else if (GetLastError() == ERROR_NOTIFY_ENUM_DIR && bytesTransferred == 0) {
        const auto& log = spdlog::get(Constants::Logging::DEFAULT_LOG);
        log->error("Detected a file change notification buffer overflow.");
    } else {
        LogLastError("Encountered error retrieving filesystem change details.");
    }
}

void WindowsFileMonitor::ProcessNotification()
{
    auto* notificationInfo =
        reinterpret_cast<FILE_NOTIFY_INFORMATION*>(m_notificationBuffer.data());

    while (notificationInfo != nullptr) {
        if (notificationInfo->FileNameLength == 0) {
            notificationInfo =
                notificationInfo->NextEntryOffset != 0
                    ? AdvancePointer(notificationInfo, notificationInfo->NextEntryOffset)
                    : nullptr;

            continue;
        }

        const auto fileNameLength = notificationInfo->FileNameLength / sizeof(wchar_t);
        Expects(fileNameLength);

        std::wstring fileName(fileNameLength, '\0');
        const auto fileSizeInBytes = notificationInfo->FileNameLength;

        // @todo Handle short filenames correctly.
        std::memcpy(&fileName[0], static_cast<void*>(notificationInfo->FileName), fileSizeInBytes);

        switch (notificationInfo->Action) {
            case FILE_ACTION_ADDED: {
                m_notificationCallback(
                    FileChangeNotification{ fileName, FileModification::CREATED });
                break;
            }
            case FILE_ACTION_REMOVED: {
                m_notificationCallback(
                    FileChangeNotification{ fileName, FileModification::DELETED });
                break;
            }
            case FILE_ACTION_MODIFIED: {
                m_notificationCallback(
                    FileChangeNotification{ fileName, FileModification::TOUCHED });
                break;
            }
            case FILE_ACTION_RENAMED_OLD_NAME: {
                m_pendingRenameEvent = std::move(fileName);
                break;
            }
            case FILE_ACTION_RENAMED_NEW_NAME: {
                Expects(m_pendingRenameEvent);

                m_notificationCallback(
                    FileChangeNotification{ fileName, FileModification::RENAMED });
                break;
            }
            default: {
                const auto& log = spdlog::get(Constants::Logging::DEFAULT_LOG);
                log->error("Encountered unknown file system event: {}.", notificationInfo->Action);
            }
        }

        notificationInfo = notificationInfo->NextEntryOffset != 0
                               ? AdvancePointer(notificationInfo, notificationInfo->NextEntryOffset)
                               : nullptr;
    }
}

#endif // Q_OS_WIN
