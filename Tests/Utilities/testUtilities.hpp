#ifndef TESTUTILITIES_HPP
#define TESTUTILITIES_HPP

#include <QtGlobal>

#include <cstdlib>
#include <filesystem>
#include <string>

namespace TestUtilities
{
    /**
     * @brief Decompressing a ZIP archive is such a pain in C++, that I'd rather make a call to a
     * Python script than try to integrate ZLib. This is just test code, after all.
     *
     * @param[in] zipFile           The path to the ZIP archive.
     * @param[in] outputDirectory   The location we'd at which we'd like to store the decompressed
     *                              data.
     */
    inline void UnzipTestData(
        const std::filesystem::path& zipFile, const std::filesystem::path& outputDirectory)
    {
        const std::string script = "../../Tests/Scripts/unzipTestData.py";

#if defined(Q_OS_WIN)
        const std::string command = "python " + script + " --input " + zipFile.string() +
                                    " --output " + outputDirectory.string();
#elif defined(Q_OS_LINUX)
        const std::string command = "python3 " + script + " --input " + zipFile.string() +
                                    " --output " + outputDirectory.string();
#endif // Q_OS_LINUX

        [[maybe_unused]] const auto result = std::system(command.c_str());
    }

    /**
     * For whatever reason, std::filesystem::absolute(...) will strip out parent directory and
     * current directory tokens on Windows, but will not on Unix platforms. That means that we'll
     * have to do it ourselves, since the scanning algorithm doesn't like these tokens.
     *
     * @param[in] unsanitizedPath   A potentially unsanitary path.
     *
     * @returns A path without parent or currently directory elements.
     */
    inline std::filesystem::path SanitizePath(const std::filesystem::path& unsanitizedPath)
    {
#if defined(Q_OS_WIN)
        constexpr auto& currentDirectory = L".";
        constexpr auto& parentDirectory = L"..";
#elif defined(Q_OS_LINUX)
        constexpr auto& currentDirectory = ".";
        constexpr auto& parentDirectory = "..";
#endif

        std::vector<std::filesystem::path::string_type> queue;
        for (const auto& token : unsanitizedPath) {
            if (token == parentDirectory && !queue.empty()) {
                queue.pop_back();
                continue;
            }

            if (token == currentDirectory) {
                continue;
            }

            queue.push_back(token);
        }

        std::filesystem::path sanitizedPath;
        for (const auto& token : queue) {
            sanitizedPath /= token;
        }

        return sanitizedPath;
    }
} // namespace TestUtilities

#endif // TESTUTILITIES_HPP
