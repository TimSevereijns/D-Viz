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

        std::system(command.c_str());
    }
} // namespace TestUtilities

#endif // TESTUTILITIES_HPP
