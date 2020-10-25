#ifndef OPERATINGSYSTEMSPECIFIC_HPP
#define OPERATINGSYSTEMSPECIFIC_HPP

#include <QClipboard>
#include <QDesktopServices>
#include <QFile>

#include "Utilities/scopeExit.hpp"
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

#endif

#ifdef Q_OS_LINUX

    inline void LaunchFileExplorer(const Tree<VizBlock>::Node& node)
    {
        const std::wstring rawPath = Controller::ResolveCompleteFilePath(node).wstring();
        const std::filesystem::path path{ rawPath };

        const auto command =
            "dbus-send --session --print-reply --dest=org.freedesktop.FileManager1 "
            "--type=method_call /org/freedesktop/FileManager1 "
            "org.freedesktop.FileManager1.ShowItems "
            "array:string:\"file://" +
            std::string{ path.c_str() } + "\" string:\"\"";

        [[maybe_unused]] const auto result = std::system(command.c_str());
    }

#endif

    inline void OpenFile(const Tree<VizBlock>::Node& node)
    {
        const auto filePath = Controller::ResolveCompleteFilePath(node);
        QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(filePath.string())));
    }

    inline void CopyFileNameToClipboard(const Tree<VizBlock>::Node& node)
    {
        const auto& file = node.GetData().file;
        const auto fileName = file.name + file.extension;

        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(QString::fromStdWString(fileName));
    }

    inline void CopyPathToClipboard(const Tree<VizBlock>::Node& node)
    {
        const auto filePath = Controller::ResolveCompleteFilePath(node);
        const auto text = QString::fromStdString(filePath.string());

        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(text);
    }

    inline bool MoveToTrash(const std::filesystem::path& filePath)
    {
        return QFile::moveToTrash(QString::fromStdString(filePath.string()));
    }
} // namespace OS

#endif // OPERATINGSYSTEMSPECIFIC_HPP
