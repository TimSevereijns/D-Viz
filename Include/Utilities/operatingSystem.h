#ifndef OPERATINGSYSTEMSPECIFIC_HPP
#define OPERATINGSYSTEMSPECIFIC_HPP

#include <QClipboard>
#include <QDesktopServices>
#include <QFile>

#include "Utilities/scopeExit.h"
#include "controller.h"

#include <filesystem>
#include <string>

#include <gsl/gsl_assert>
#include <spdlog/spdlog.h>

#ifdef Q_OS_WIN
#include <ShlObj.h>
#include <objbase.h>
#endif

#ifdef Q_OS_LINUX
#include <sys/statvfs.h>
#endif

namespace OS
{
#ifdef Q_OS_WIN
    inline bool LaunchFileExplorer(const std::filesystem::path& path)
    {
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        const ScopeExit onScopeExit = [&]() noexcept
        {
            CoUninitialize();
        };

        const auto pathAsString = path.wstring();
        Expects(std::none_of(
            std::begin(pathAsString), std::end(pathAsString), [](const auto character) {
                return character == L'/'; //< Certain WinAPI functions can't deal with this slash.
            }));

        bool openedSuccessfully = false;

        auto* const idList = ILCreateFromPath(pathAsString.c_str());
        if (idList) {
            openedSuccessfully = (SHOpenFolderAndSelectItems(idList, 0, nullptr, 0) == S_OK);
            ILFree(idList);
        }

        return openedSuccessfully;
    }

#endif

#ifdef Q_OS_LINUX

    inline bool LaunchFileExplorer(const std::filesystem::path& path)
    {
        const auto command =
            "dbus-send --session --print-reply --dest=org.freedesktop.FileManager1 "
            "--type=method_call /org/freedesktop/FileManager1 "
            "org.freedesktop.FileManager1.ShowItems "
            "array:string:\"file://" +
            std::string{ path.c_str() } + "\" string:\"\"";

        const auto exitCode = std::system(command.c_str());
        return exitCode == 0;
    }

#endif

    inline void OpenFile(const std::filesystem::path& path)
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(path.string())));
    }

    inline void CopyFileNameToClipboard(const std::filesystem::path& path)
    {
        const auto fileName =
            QString::fromStdWString(path.filename().wstring() + path.extension().wstring());

        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(fileName);
    }

    inline void CopyPathToClipboard(const std::filesystem::path& path)
    {
        const auto fileName = QString::fromStdWString(path.wstring());

        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(fileName);
    }

    inline bool MoveToTrash(const std::filesystem::path& filePath)
    {
        return QFile::moveToTrash(QString::fromStdString(filePath.string()));
    }
} // namespace OS

#endif // OPERATINGSYSTEMSPECIFIC_HPP
