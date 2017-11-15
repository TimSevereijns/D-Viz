#ifndef OPERATINGSYSTEMSPECIFIC_HPP
#define OPERATINGSYSTEMSPECIFIC_HPP

#include "Utilities/ignoreUnused.hpp"
#include "Utilities/scopeExit.hpp"

#include <string>
#include <experimental/filesystem>

#include <spdlog/spdlog.h>

#ifdef Q_OS_WIN
   #include <ShlObj.h>
   #include <Objbase.h>
#endif

#ifdef Q_OS_LINUX
   #include <sys/statvfs.h>
#endif

/**
 * @note Any functions defined in this namespace will have to be declared 'inline', since we're not
 * splitting definitions from declarations. The 'inline' keyword will direct the linker to allow
 * multiple definitions of a given function.
 */
namespace OperatingSystemSpecific
{
#ifdef Q_OS_WIN

   static constexpr auto PREFERRED_SLASH = std::experimental::filesystem::path::preferred_separator;

   inline void LaunchFileExplorer(const Tree<VizFile>::Node& node)
   {
      CoInitializeEx(NULL, COINIT_MULTITHREADED);
      ON_SCOPE_EXIT noexcept { CoUninitialize(); };

      const std::wstring filePath = Controller::ResolveCompleteFilePath(node);

      assert(std::none_of(std::begin(filePath), std::end(filePath),
         [] (const auto character)
      {
         return character == L'/';
      }));

      auto* const idList = ILCreateFromPath(filePath.c_str());
      if (idList)
      {
         SHOpenFolderAndSelectItems(idList, 0, 0, 0);
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
         path.c_str(),
         NULL,
         (PULARGE_INTEGER)&totalNumberOfBytes,
         (PULARGE_INTEGER)&totalNumberOfFreeBytes);

      assert(wasOperationSuccessful);

      const auto& log = spdlog::get(Constants::Logging::DEFAULT_LOG);
      log->info(fmt::format("Disk Size:  {} bytes", totalNumberOfBytes));
      log->info(fmt::format("Free Space: {} bytes", totalNumberOfFreeBytes));

      const auto occupiedSpace = totalNumberOfBytes - totalNumberOfFreeBytes;
      return occupiedSpace;
   }

#endif

#ifdef Q_OS_LINUX

   static constexpr auto PREFERRED_SLASH =
   static_cast<wchar_t>(std::experimental::filesystem::path::preferred_separator);

   inline void LaunchFileExplorer(const Tree<VizFile>::Node& node)
   {
      const std::wstring rawPath = Controller::ResolveCompleteFilePath(node);
      const std::experimental::filesystem::path path{ rawPath };

      // @todo Look into adding support for other popular file browsers, like Nautilus.

      fmt::MemoryWriter writer;
      writer << "nemo \"" << path.c_str() << "\"";

      const auto result = std::system(writer.c_str());
      IgnoreUnused(result);
   }

   inline std::uint64_t GetUsedDiskSpace(const std::wstring& rawPath)
   {
      const std::experimental::filesystem::path path{ rawPath };

      struct statvfs diskInfo;
      statvfs(path.string().data(), &diskInfo);

      const auto totalNumberOfBytes = diskInfo.f_blocks * diskInfo.f_bsize;
      const auto totalNumberOfFreeBytes = diskInfo.f_bfree * diskInfo.f_bsize;

      const auto& log = spdlog::get(Constants::Logging::DEFAULT_LOG);
      log->info(fmt::format("Disk Size:  {} bytes", totalNumberOfBytes));
      log->info(fmt::format("Free Space: {} bytes", totalNumberOfFreeBytes));

      return totalNumberOfBytes - totalNumberOfFreeBytes;
   }

#endif
}

#endif // OPERATINGSYSTEMSPECIFIC_HPP
