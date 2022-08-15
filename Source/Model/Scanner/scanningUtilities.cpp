#include "Model/Scanner/scanningUtilities.h"
#include "Model/vizBlock.h"
#include "Utilities/ignoreUnused.h"

#include <QtGlobal>

#ifdef Q_OS_WIN
#include <fileapi.h>
#include <winioctl.h>

#include "Utilities/reparsePointDeclarations.h"
#endif // Q_OS_WIN

#include "constants.h"

#include <memory>
#include <mutex>

#include <Tree/Tree.hpp>
#include <gsl/assert>
#include <spdlog/spdlog.h>

namespace Scanner
{
    std::uintmax_t GetFileSizeUsingWinAPI(const std::filesystem::path& path) noexcept
    {
        IgnoreUnused(path);

        std::uintmax_t fileSize = 0;

#ifdef Q_OS_WIN
        WIN32_FIND_DATA fileData;
        const HANDLE fileHandle = FindFirstFileW(path.wstring().data(), &fileData);
        if (fileHandle == INVALID_HANDLE_VALUE) // NOLINT
        {
            return 0;
        }

        const auto highWord = static_cast<std::uintmax_t>(fileData.nFileSizeHigh);
        fileSize = (highWord << sizeof(fileData.nFileSizeLow) * 8) | fileData.nFileSizeLow;
        FindClose(fileHandle);
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
            if (parentInfo.type == FileType::Directory) {
                parentInfo.size += fileInfo.size;
            }
        }
    }

#ifdef Q_OS_WIN
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
