#ifndef OPERATINGSYSTEMSPECIFIC_HPP
#define OPERATINGSYSTEMSPECIFIC_HPP

#include "Utilities/ignoreUnused.hpp"
#include "Utilities/scopeExit.hpp"
#include "controller.h"

#include <filesystem>
#include <string>

#include <gsl/gsl_assert>
#include <spdlog/spdlog.h>

#ifdef Q_OS_WIN
#include <Objbase.h>
#include <ShlObj.h>
#endif

#ifdef Q_OS_LINUX
#include <sys/statvfs.h>
#endif

namespace OS
{
#ifdef Q_OS_WIN

    inline void LaunchFileExplorer(const Tree<VizBlock>::Node& node)
    {
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        const ScopeExit onScopeExit = [&]() noexcept
        {
            CoUninitialize();
        };

        const std::wstring filePath = Controller::ResolveCompleteFilePath(node);

        Expects(std::none_of(std::begin(filePath), std::end(filePath), [](const auto character) {
            return character == L'/'; //< Certain Windows API functions can't deal with this slash.
        }));

        auto* const idList = ILCreateFromPath(filePath.c_str());
        if (idList) {
            SHOpenFolderAndSelectItems(idList, 0, nullptr, 0);
            ILFree(idList);
        }
    }

    inline std::uint64_t GetUsedDiskSpace(std::wstring path)
    {
        std::replace(std::begin(path), std::end(path), L'/', L'\\');
        path += '\\';

        std::uint64_t totalNumberOfFreeBytes{ 0 };
        std::uint64_t totalNumberOfBytes{ 0 };
        const bool wasOperationSuccessful = GetDiskFreeSpaceExW(
            path.c_str(), NULL, (PULARGE_INTEGER)&totalNumberOfBytes,
            (PULARGE_INTEGER)&totalNumberOfFreeBytes);

        Expects(wasOperationSuccessful);

        const auto& log = spdlog::get(Constants::Logging::DefaultLog);
        log->info(fmt::format("Disk Size:  {} bytes", totalNumberOfBytes));
        log->info(fmt::format("Free Space: {} bytes", totalNumberOfFreeBytes));

        const auto occupiedSpace = totalNumberOfBytes - totalNumberOfFreeBytes;
        return occupiedSpace;
    }

#endif

#ifdef Q_OS_LINUX

    inline void LaunchFileExplorer(const Tree<VizBlock>::Node& node)
    {
        const std::wstring rawPath = Controller::ResolveCompleteFilePath(node).wstring();
        const std::filesystem::path path{ rawPath };

        // @todo Look into adding support for other popular file browsers, like Nautilus.

        const auto message = "nemo \"" + std::string{ path.c_str() } + "\" &";
        const auto result = std::system(message.c_str());
        IgnoreUnused(result);
    }

    inline std::uint64_t GetUsedDiskSpace(const std::wstring& rawPath)
    {
        const std::filesystem::path path{ rawPath };

        struct statvfs diskInfo;
        statvfs(path.string().data(), &diskInfo);

        const auto totalNumberOfBytes = diskInfo.f_blocks * diskInfo.f_bsize;
        const auto totalNumberOfFreeBytes = diskInfo.f_bfree * diskInfo.f_bsize;

        const auto& log = spdlog::get(Constants::Logging::DefaultLog);
        log->info(fmt::format("Disk Size:  {} bytes", totalNumberOfBytes));
        log->info(fmt::format("Free Space: {} bytes", totalNumberOfFreeBytes));

        return totalNumberOfBytes - totalNumberOfFreeBytes;
    }

#endif
} // namespace OS

#endif // OPERATINGSYSTEMSPECIFIC_HPP
