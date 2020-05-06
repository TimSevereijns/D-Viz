#include "Monitor/windowsFileMonitor.h"

#ifdef Q_OS_WIN

#include <Utilities/scopeExit.hpp>

#include <exception>
#include <string_view>

#include <gsl/gsl_assert>
#include <spdlog/spdlog.h>

#include "Utilities/scopedHandle.h"
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
     * @brief Retrieves the last error and converts it to a string.
     *
     * @returns The System Error Code.
     */
    std::string GetLastErrorAsString()
    {
        DWORD errorMessageID = ::GetLastError();
        if (errorMessageID == 0) {
            return {};
        }

        LPSTR messageBuffer = nullptr;
        const ScopeExit onScopeExit = [&]() noexcept
        {
            ::LocalFree(messageBuffer);
        };

        constexpr auto formattingOptions = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                           FORMAT_MESSAGE_FROM_SYSTEM |
                                           FORMAT_MESSAGE_IGNORE_INSERTS;

        const auto characterCount = ::FormatMessageA(
            /* dwFlags = */ formattingOptions,
            /* lpSource = */ nullptr,
            /* dwMessageId = */ errorMessageID,
            /* dwLanguageId = */ MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            /* lpBuffer = */ reinterpret_cast<LPSTR>(&messageBuffer),
            /* nSize = */ 0,
            /* Arguments = */ nullptr);

        return std::string(messageBuffer, characterCount);
    }

    /**
     * @brief Converts the numeric value of the last error to a string and logs it.
     *
     * @param[in] message           The message to log.
     */
    void LogLastError(std::string_view message)
    {
        const auto lastError = GetLastErrorAsString();
        const auto& log = spdlog::get(Constants::Logging::DefaultLog);
        log->error("{} Last Error: {}.", message, lastError);
    }

    /**
     * @brief Advances the notification struct to the next message.
     *
     * @param[in, out] notificationInfo   The notification info object.
     */
    void AdvanceToNextNotification(FILE_NOTIFY_INFORMATION*& notificationInfo)
    {
        if (notificationInfo->NextEntryOffset) {
            notificationInfo = AdvancePointer(notificationInfo, notificationInfo->NextEntryOffset);
        } else {
            notificationInfo = nullptr;
        }
    }
} // namespace

WindowsFileMonitor::~WindowsFileMonitor() noexcept
{
    if (m_isActive.load()) {
        Stop();
    }
}

bool WindowsFileMonitor::IsActive() const
{
    return m_isActive.load();
}

void WindowsFileMonitor::Start(
    const std::filesystem::path& path, std::function<void(FileEvent&&)> onNotificationCallback)
{
    m_pathBeingMonitored = path;
    m_notificationCallback = std::move(onNotificationCallback);

    // When monitoring a file on a network drive, the size of the buffer cannot exceed 64 KiB.
    // To quote the documentation: "This is due to a packet size limitation with the underlying file
    // sharing protocols." In the C# documentation for the analogous `FileSystemWatcher`, Microsoft
    // appears to default the buffer's size to 8,192 bytes (or 8 KiB), so we'll do the same.
    // Interestingly, a small buffer can also be significantly faster:
    // https://randomascii.wordpress.com/2018/04/17/making-windows-slower-part-1-file-access/

    using namespace Literals::Numeric::Binary;
    m_notificationBuffer.resize(8_KiB, std::byte{ 0 });

    m_fileHandle = ::CreateFileW(
        /* lpFileName = */ path.wstring().data(),
        /* dwDesiredAccess = */ FILE_LIST_DIRECTORY | STANDARD_RIGHTS_READ,
        /* dwShareMode = */ FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        /* lpSecurityAttributes = */ nullptr,
        /* dwCreationDisposition = */ OPEN_EXISTING,
        /* dwFlagsAndAttributes = */ FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        /* hTemplateFile = */ nullptr);

    if (!m_fileHandle || m_fileHandle == INVALID_HANDLE_VALUE) {
        const auto& log = spdlog::get(Constants::Logging::DefaultLog);
        log->error("Could not acquire handle to: {}.", path.string());

        throw std::runtime_error{ "File monitoring failed to start." };
    }

    const auto exitThreadHandle = ::CreateEventW(
        /* lpEventAttributes = */ nullptr,
        /* bManualReset = */ true,
        /* bInitialState = */ false,
        /* lpName = */ L"D-VIZ_FILE_MONITOR_TERMINATE_THREAD");

    m_events.SetExitHandle(exitThreadHandle);

    const auto notificationHandle = ::CreateEventW(
        /* lpEventAttributes = */ nullptr,
        /* bManualReset = */ false,
        /* bInitialState = */ false,
        /* lpName = */ L"D-VIZ_FILE_MONITOR_NOTIFICATION");

    m_events.SetNotificationHandle(notificationHandle);

    ::ZeroMemory(&m_ioBuffer, sizeof(OVERLAPPED));
    m_ioBuffer.hEvent = notificationHandle;

    m_isActive.store(true);
    m_monitoringThread = std::thread{ [&] { Monitor(); } };
}

void WindowsFileMonitor::Stop()
{
    ::SetEvent(m_events.GetExitHandle());

    if (m_monitoringThread.joinable()) {
        m_monitoringThread.join();
    }

    Expects(m_isActive == false);

    if (m_fileHandle && m_fileHandle != INVALID_HANDLE_VALUE) {
        ::CloseHandle(m_fileHandle);
    }
}

void WindowsFileMonitor::ShutdownThread()
{
    m_keepMonitoring.store(false);

    if (!std::filesystem::exists(m_pathBeingMonitored)) {
        // @note If the path being monitored now longer exists (for whatever reason), then we can't
        // cancel I/O operations on it. So, if that happens, we'll just bail.

        Expects(false);
        return;
    }

    ::CancelIo(m_fileHandle);

    while (!HasOverlappedIoCompleted(&m_ioBuffer)) {
        ::SleepEx(50, TRUE);
    }
}

void WindowsFileMonitor::Monitor()
{
    while (m_keepMonitoring) {
        AwaitNotification();
    }

    m_isActive.store(false);
}

void WindowsFileMonitor::AwaitNotification()
{
    constexpr auto desiredNotifications = FILE_NOTIFY_CHANGE_FILE_NAME |
                                          FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_SIZE |
                                          FILE_NOTIFY_CHANGE_CREATION;

    const bool successfullyQueued = ::ReadDirectoryChangesW(
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

    const auto waitResult = ::WaitForMultipleObjects(
        /* nCount = */ m_events.Size(),
        /* lpHandles = */ m_events.Data(),
        /* bWaitAll = */ false,
        /* dwMilliseconds = */ INFINITE);

    switch (waitResult) {
        case WAIT_OBJECT_0: {
            ShutdownThread();
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

    const bool successfullyRead = ::GetOverlappedResult(
        /* hFile = */ m_fileHandle,
        /* lpOverlapped = */ &m_ioBuffer,
        /* lpNumberOfBytesTransferred = */ &bytesTransferred,
        /* bWait = */ false);

    if (successfullyRead && bytesTransferred > 0) {
        ProcessNotification();
    } else if (GetLastError() == ERROR_NOTIFY_ENUM_DIR && bytesTransferred == 0) {
        const auto& log = spdlog::get(Constants::Logging::DefaultLog);
        log->error(
            "Detected a file change notification buffer overflow. This means that too many file "
            "changes occurred at once, and some change notifications may have been missed as a "
            "result.");
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
            AdvanceToNextNotification(notificationInfo);
            continue;
        }

        const auto fileNameLength = notificationInfo->FileNameLength / sizeof(wchar_t);
        Expects(fileNameLength);

        std::wstring fileName(fileNameLength, '\0');

        // @todo Handle short filenames correctly.

        const auto fileSizeInBytes = notificationInfo->FileNameLength;
        std::memcpy(&fileName[0], static_cast<void*>(notificationInfo->FileName), fileSizeInBytes);

        switch (notificationInfo->Action) {
            case FILE_ACTION_ADDED: {
                m_notificationCallback(FileEvent{ fileName, FileEventType::Created });
                break;
            }
            case FILE_ACTION_REMOVED: {
                m_notificationCallback(FileEvent{ fileName, FileEventType::Deleted });
                break;
            }
            case FILE_ACTION_MODIFIED: {
                m_notificationCallback(FileEvent{ fileName, FileEventType::Touched });
                break;
            }
            case FILE_ACTION_RENAMED_OLD_NAME: {
                m_pendingRenameEvent = std::move(fileName);
                break;
            }
            case FILE_ACTION_RENAMED_NEW_NAME: {
                m_notificationCallback(FileEvent{ fileName, FileEventType::Renamed });
                break;
            }
            default: {
                const auto& log = spdlog::get(Constants::Logging::DefaultLog);
                log->error("Encountered unknown file system event: {}.", notificationInfo->Action);
            }
        }

        AdvanceToNextNotification(notificationInfo);
    }
}

#endif // Q_OS_WIN
