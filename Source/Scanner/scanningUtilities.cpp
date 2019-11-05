#include "Scanner/scanningUtilities.h"
#include "Utilities/ignoreUnused.hpp"
#include "Visualizations/vizBlock.h"

#include <QtGlobal>

#ifdef Q_OS_WIN
#include <fileapi.h>
#include <winioctl.h>

#include "Utilities/reparsePointDeclarations.hpp"
#endif // Q_OS_WIN

#include "constants.h"

#include <memory>
#include <mutex>

#include <Tree/Tree.hpp>
#include <gsl/gsl_assert>
#include <spdlog/spdlog.h>

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
            const auto& log = spdlog::get(Constants::Logging::DefaultLog);
            log->warn("Falling back on the Win API for: \"{}\".", path.string());

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
                   ? reinterpret_cast<REPARSE_DATA_BUFFER*>(buffer.data())->ReparseTag == targetTag
                   : false;
    }

    bool IsReparsePoint(const std::filesystem::path& path) noexcept
    {
        WIN32_FIND_DATA findData;
        HANDLE handle = FindFirstFile(path.native().c_str(), &findData);
        if (handle == INVALID_HANDLE_VALUE) {
            return false;
        }

        const auto attributes = findData.dwFileAttributes;
        if ((attributes & FILE_ATTRIBUTE_REPARSE_POINT) &&
            (attributes & FILE_ATTRIBUTE_DIRECTORY)) {
            FindClose(handle);
            return true;
        }

        FindClose(handle);
        return false;
    }
#endif // Q_OS_WIN
} // namespace Scanner
