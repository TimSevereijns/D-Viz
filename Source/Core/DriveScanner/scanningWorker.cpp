#include "driveScanner.h"
#include "driveScanningUtilities.h"

#include "scanningWorker.h"

#ifdef Q_OS_WIN
#pragma warning(push)
#pragma warning(disable : 4996)
#endif // Q_OS_WIN

#include <boost/asio/post.hpp>

#ifdef Q_OS_WIN
#pragma warning(pop)
#endif // Q_OS_WIN

#include <Stopwatch/Stopwatch.hpp>
#include <spdlog/spdlog.h>

#include "../constants.h"

namespace
{
    /**
     * @brief Removes nodes whose corresponding file or directory size is zero. This is often
     * necessary because a directory may contain only a single other directory within it that is
     * empty. In such a case, the outer directory has a size of zero, but
     * std::experimental::filesystem::is_empty will still have reported this directory as being
     * non-empty.
     *
     * @param[in, out] tree           The tree to be pruned.
     */
    void PruneEmptyFilesAndDirectories(Tree<VizBlock>& tree) noexcept
    {
        std::vector<Tree<VizBlock>::Node*> toBeDeleted;

        for (auto&& node : tree) {
            if (node->file.size == 0) {
                toBeDeleted.emplace_back(&node);
            }
        }

        const auto nodesRemoved = toBeDeleted.size();

        for (auto* node : toBeDeleted) {
            node->DeleteFromTree();
        }

        spdlog::get(Constants::Logging::DEFAULT_LOG)
            ->info(fmt::format("Number of Sizeless Files Removed: {}", nodesRemoved));
    }

    /**
     * @brief Contructs the root node for the file tree.
     *
     * @param[in] path                The path to the directory that should constitute the root
     * node.
     */
    std::shared_ptr<Tree<VizBlock>>
    CreateTreeAndRootNode(const std::experimental::filesystem::path& path) noexcept
    {
        if (!std::experimental::filesystem::is_directory(path)) {
            return nullptr;
        }

        FileInfo fileInfo{ path.wstring(),
                           /* extension = */ L"", ScanningWorker::SIZE_UNDEFINED,
                           FileType::DIRECTORY };

        return std::make_shared<Tree<VizBlock>>(VizBlock{ std::move(fileInfo) });
    }

    /**
     * @returns True if the directory should be processed.
     */
    auto ShouldProcess(const std::experimental::filesystem::path& path) noexcept
    {
#if defined(Q_OS_WIN)
        return !DriveScanning::Utilities::IsReparsePoint(path);
#elif defined(Q_OS_LINUX)
        return !std::experimental::filesystem::is_symlink(path);
#endif // Q_OS_LINUX
    }
} // namespace

ScanningWorker::ScanningWorker(
    const DriveScanningParameters& parameters, ScanningProgress& progress)
    : m_parameters{ parameters },
      m_progress{ progress },
      m_fileTree{ CreateTreeAndRootNode(parameters.path) }
{
}

void ScanningWorker::ProcessFile(
    const std::experimental::filesystem::path& path, Tree<VizBlock>::Node& treeNode) noexcept
{
    const auto fileSize = DriveScanning::Utilities::ComputeFileSize(path);
    if (fileSize == 0u) {
        return;
    }

    m_progress.bytesProcessed.fetch_add(fileSize);
    m_progress.filesScanned.fetch_add(1);

    FileInfo fileInfo{ path.filename().stem().wstring(), path.filename().extension().wstring(),
                       fileSize, FileType::REGULAR };

    std::unique_lock<decltype(m_mutex)> lock{ m_mutex };
    treeNode.AppendChild(VizBlock{ std::move(fileInfo) });
}

void ScanningWorker::ProcessDirectory(
    const std::experimental::filesystem::path& path, Tree<VizBlock>::Node& node) noexcept
{
    auto isRegularFile{ false };
    try {
        // In certain cases, this function can, apparently, raise exceptions, although it isn't
        // entirely clear to me what circumstances need to exist for this to occur:
        isRegularFile = std::experimental::filesystem::is_regular_file(path);
    } catch (...) {
        return;
    }

    if (isRegularFile) {
        ProcessFile(path, node);
    } else if (std::experimental::filesystem::is_directory(path) && ShouldProcess(path)) {
        try {
            // In some edge-cases, the Windows operating system doesn't allow anyone to access
            // certain directories, and attempts to do so will result in exceptional behaviour---pun
            // intended. In order to deal with these rare cases, we'll need to rely on a try-catch
            // to keep going. One example of a problematic directory in Windows 7 is: "C:\System
            // Volume Information".
            if (std::experimental::filesystem::is_empty(path)) {
                return;
            }
        } catch (...) {
            return;
        }

        FileInfo directoryInfo{ path.filename().wstring(),
                                /* extension = */ L"", ScanningWorker::SIZE_UNDEFINED,
                                FileType::DIRECTORY };

        std::unique_lock<decltype(m_mutex)> lock{ m_mutex };
        auto* const lastChild = node.AppendChild(VizBlock{ std::move(directoryInfo) });
        lock.unlock();

        m_progress.directoriesScanned.fetch_add(1);

        AddSubDirectoriesToQueue(path, *lastChild);
    }
}

void ScanningWorker::AddSubDirectoriesToQueue(
    const std::experimental::filesystem::path& path, Tree<VizBlock>::Node& node) noexcept
{
    auto itr = std::experimental::filesystem::directory_iterator{ path };
    const auto end = std::experimental::filesystem::directory_iterator{};

    while (itr != end) {
        boost::asio::post(
            m_threadPool, [&, path = itr->path() ]() noexcept { ProcessDirectory(path, node); });

        ++itr;
    }
}

void ScanningWorker::Start()
{
    emit ProgressUpdate();

    Stopwatch<std::chrono::seconds>(
        [&]() noexcept {
            boost::asio::post(m_threadPool, [&]() noexcept {
                AddSubDirectoriesToQueue(m_parameters.path, *m_fileTree->GetRoot());
            });

            m_threadPool.join();
        },
        [](const auto& elapsed, const auto& units) noexcept {
            spdlog::get(Constants::Logging::DEFAULT_LOG)
                ->info(fmt::format("Scanned Drive in: {} {}", elapsed.count(), units));
        });

    DriveScanning::Utilities::ComputeDirectorySizes(*m_fileTree);
    PruneEmptyFilesAndDirectories(*m_fileTree);

    emit Finished(m_fileTree);
}
