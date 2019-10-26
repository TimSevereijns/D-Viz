#include "Scanner/scanningUtilities.h"
#include "Utilities/ignoreUnused.hpp"
#include "Visualizations/vizBlock.h"

#include <QtGlobal>

#ifdef Q_OS_WIN
#include <fileapi.h>
#include <winioctl.h>

#include "Utilities/reparsePointDeclarations.hpp"
#endif // Q_OS_WIN

#include <iostream>
#include <memory>
#include <mutex>

#include <Tree/Tree.hpp>
#include <gsl/gsl_assert>

namespace
{
    std::mutex streamMutex;
}

namespace Scanner
{
#ifdef Q_OS_WIN

    ScopedHandle OpenReparsePoint(const std::filesystem::path& path) noexcept
    {
        const auto handle = CreateFile(
            /* lpFileName = */ path.wstring().c_str(),
            /* dwDesiredAccess = */ GENERIC_READ,
            /* dwShareMode = */ 0,
            /* lpSecurityAttributes = */ nullptr,
            /* dwCreationDisposition = */ OPEN_EXISTING,
            /* dwFlagsAndAttributes = */ FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
            /* hTemplateFile = */ nullptr);

        return ScopedHandle{ handle };
    }

    bool ReadReparsePoint(const std::wstring& path, std::vector<std::byte>& reparseBuffer) noexcept
    {
        const auto handle = OpenReparsePoint(path);
        if (!handle.IsValid()) {
            return false;
        }

        DWORD bytesReturned{ 0 };

        const auto successfullyRetrieved =
            DeviceIoControl(
                /* hDevice = */ static_cast<HANDLE>(handle),
                /* dwIoControlCode = */ FSCTL_GET_REPARSE_POINT,
                /* lpInBuffer = */ nullptr,
                /* nInBufferSize = */ 0,
                /* lpOutBuffer = */ static_cast<LPVOID>(reparseBuffer.data()),
                /* nOutBufferSize = */ static_cast<DWORD>(reparseBuffer.size()),
                /* lpBytesReturned = */ &bytesReturned,
                /* lpOverlapped = */ nullptr) == TRUE;

        return successfullyRetrieved && bytesReturned;
    }

#endif // Q_OS_WIN

    std::uintmax_t GetFileSizeUsingWinAPI(const std::filesystem::path& path) noexcept
    {
        std::uintmax_t fileSize{ 0 };

        IgnoreUnused(path);

#ifdef Q_OS_WIN

        WIN32_FIND_DATA fileData;
        const HANDLE fileHandle = FindFirstFileW(path.wstring().data(), &fileData);
        if (fileHandle == INVALID_HANDLE_VALUE) // NOLINT
        {
            return 0;
        }

        const auto highWord = static_cast<std::uintmax_t>(fileData.nFileSizeHigh);
        fileSize = (highWord << sizeof(fileData.nFileSizeLow) * 8) | fileData.nFileSizeLow;

#endif // Q_OS_WIN

        return fileSize;
    }

    std::uintmax_t ComputeFileSize(const std::filesystem::path& path) noexcept
    {
        try {
            Expects(std::filesystem::is_directory(path) == false);

            return std::filesystem::file_size(path);
        } catch (...) {
            std::lock_guard<std::mutex> lock{ streamMutex };
            IgnoreUnused(lock);

            std::wcout << "Falling back on the Win API for: \"" << path.wstring() << "\""
                       << std::endl;

            return GetFileSizeUsingWinAPI(path);
        }
    }

    void ComputeDirectorySizes(Tree<VizBlock>& tree) noexcept
    {
        for (auto&& node : tree) {
            const auto& fileInfo = node->file;

            auto* parent = node.GetParent();
            if (!parent) {
                return;
            }

            auto& parentInfo = parent->GetData().file;
            if (parentInfo.type == FileType::DIRECTORY) {
                parentInfo.size += fileInfo.size;
            }
        }
    }

#ifdef Q_OS_WIN

    bool IsReparseTag(const std::filesystem::path& path, DWORD targetTag) noexcept
    {
        static std::vector<std::byte> buffer{ MAXIMUM_REPARSE_DATA_BUFFER_SIZE };

        const auto successfullyRead = ReadReparsePoint(path, buffer);

        return successfullyRead
                   ? reinterpret_cast<REPARSE_DATA_BUFFER*>(buffer.data())->ReparseTag ==
                         targetTag // NOLINT
                   : false;
    }

    bool IsMountPoint(const std::filesystem::path& path) noexcept
    {
        const auto isMountPoint = IsReparseTag(path, IO_REPARSE_TAG_MOUNT_POINT);
        if (isMountPoint) {
            const std::lock_guard<decltype(streamMutex)> lock{ streamMutex };
            IgnoreUnused(lock);

            std::wcout << L"Found Mount Point: " << path.wstring() << std::endl;
        }

        return isMountPoint;
    }

    bool IsSymlink(const std::filesystem::path& path) noexcept
    {
        const auto isSymlink = IsReparseTag(path, IO_REPARSE_TAG_SYMLINK);
        if (isSymlink) {
            const std::lock_guard<decltype(streamMutex)> lock{ streamMutex };
            IgnoreUnused(lock);

            std::wcout << L"Found Symlink: " << path.wstring() << std::endl;
        }

        return isSymlink;
    }

    bool IsReparsePoint(const std::filesystem::path& path) noexcept
    {
        const auto handle = OpenReparsePoint(path);
        if (!handle.IsValid()) {
            return false;
        }

        BY_HANDLE_FILE_INFORMATION fileInfo;

        const auto successfullyRetrieved =
            GetFileInformationByHandle(static_cast<HANDLE>(handle), &fileInfo);

        if (!successfullyRetrieved) {
            return false;
        }

        return fileInfo.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT;
    }

#endif // Q_OS_WIN
} // namespace Scanner
