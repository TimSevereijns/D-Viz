#include "Model/Scanner/scanningWorker.h"

#include "Model/Scanner/scanningUtilities.h"
#include "constants.h"

#ifdef Q_OS_WIN
#pragma warning(push)
#pragma warning(disable : 4996)
#endif // Q_OS_WIN

#include <boost/asio/post.hpp>

#ifdef Q_OS_WIN
#pragma warning(pop)
#endif // Q_OS_WIN

#include <spdlog/spdlog.h>
#include <stopwatch.h>

namespace
{
    /**
     * @brief Removes nodes whose corresponding file or directory size is zero. This is often
     * necessary because a directory may contain only a single other directory within it that is
     * empty. In such a case, the outer directory has a size of zero, but std::filesystem::is_empty
     * will still have reported this directory as being non-empty.
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

        const auto& log = spdlog::get(Constants::Logging::DefaultLog);
        log->info("Number of Sizeless Files Removed: {:L}", nodesRemoved);
    }

    /**
     * @brief Contructs the root node for the file tree.
     *
     * @param[in] path                The path to the directory that should constitute the root
     * node.
     */
    std::shared_ptr<Tree<VizBlock>>
    CreateTreeAndRootNode(const std::filesystem::path& path) noexcept
    {
        if (!std::filesystem::is_directory(path)) {
            return nullptr;
        }

        constexpr auto noExtension = "";
        FileInfo fileInfo{ path.string(), noExtension, ScanningWorker::UndefinedFileSize,
                           FileType::Directory };

        return std::make_shared<Tree<VizBlock>>(VizBlock{ std::move(fileInfo) });
    }

    /**
     * @brief Detects path elements that will cause infinite looping.
     *
     * During testing, I ran across a directory containing files whose path contained either
     * a single dot (representing the current directory), or two dots (representing the parent
     * directory). The presense of these path elements caused the scanning logic to loop
     * indefinitely.
     *
     * @param[in] path              The path to test.
     *
     * @returns True if a problematic element is detected.
     */
    bool ContainsProblematicPathElements(const std::filesystem::path& path) noexcept
    {
        for (const auto& entry : path) {
            const auto& data = entry.native();
#if defined(Q_OS_WIN)
            if (data == L".." || data == L".") {
#elif defined(Q_OS_LINUX)
            if (data == ".." || data == ".") {
#endif // Q_OS_LINUX
                return true;
            }
        }

        return false;
    }
} // namespace

ScanningWorker::ScanningWorker(
    const ScanningOptions& options, ScanningProgress& progress,
    std::atomic<bool>& cancellationToken)
    : m_options{ options },
      m_progress{ progress },
      m_cancellationToken{ cancellationToken },
      m_fileTree{ CreateTreeAndRootNode(options.path) }
{
}

bool ScanningWorker::IsScannable(const std::filesystem::path& path) noexcept
{
#if defined(Q_OS_WIN)
    return !Scanner::IsReparsePoint(path);
#elif defined(Q_OS_LINUX)
    return !std::filesystem::is_symlink(path);
#endif // Q_OS_LINUX
}

void ScanningWorker::ProcessFile(
    const std::filesystem::path& path, Tree<VizBlock>::Node& treeNode) noexcept
{
    const auto fileSize = Scanner::ComputeFileSize(path);
    if (fileSize == 0u) {
        return;
    }

    m_progress.bytesProcessed.fetch_add(fileSize);
    m_progress.filesScanned.fetch_add(1);

    FileInfo fileInfo{ path, fileSize, FileType::Regular };

    std::unique_lock<decltype(m_mutex)> lock{ m_mutex };
    treeNode.AppendChild(VizBlock{ std::move(fileInfo) });
}

void ScanningWorker::ProcessPath(
    const std::filesystem::path& path, Tree<VizBlock>::Node& node) noexcept
{
    if (ContainsProblematicPathElements(path) || m_cancellationToken.load()) {
        return;
    }

    auto isRegularFile = false;
    try {
        // In certain cases, this function can, apparently, raise exceptions, although it isn't
        // entirely clear to me what circumstances need to exist for this to occur:
        isRegularFile = std::filesystem::is_regular_file(path);
    } catch (...) {
        return;
    }

    if (isRegularFile) {
        ProcessFile(path, node);
    } else if (std::filesystem::is_directory(path) && IsScannable(path)) {
        try {
            // In some edge-cases, the Windows operating system doesn't allow anyone to access
            // certain directories, and attempts to do so will result in exceptional behaviour---pun
            // intended. In order to deal with these rare cases, we'll need to rely on a try-catch
            // to keep going. One example of a problematic directory in Windows 7 is: "C:\System
            // Volume Information".
            if (std::filesystem::is_empty(path)) {
                return;
            }
        } catch (...) {
            return;
        }

        constexpr auto emptyExtension = "";
        FileInfo directoryInfo{ path.filename().string(), emptyExtension,
                                ScanningWorker::UndefinedFileSize, FileType::Directory };

        std::unique_lock<decltype(m_mutex)> lock{ m_mutex };
        auto* const lastChild = node.AppendChild(VizBlock{ std::move(directoryInfo) });
        lock.unlock();

        m_progress.directoriesScanned.fetch_add(1);

        AddSubDirectoriesToQueue(path, *lastChild);
    }
}

void ScanningWorker::AddSubDirectoriesToQueue(
    const std::filesystem::path& path, Tree<VizBlock>::Node& node) noexcept
{
    auto itr = std::filesystem::directory_iterator{ path };
    const auto end = std::filesystem::directory_iterator{};

    while (itr != end) {
        boost::asio::post(
            m_threadPool, [&, path = itr->path() ]() noexcept { ProcessPath(path, node); });

        ++itr;
    }
}

void ScanningWorker::Start()
{
    emit ProgressUpdate();

    const auto stopwatch = Stopwatch<std::chrono::seconds>([&]() noexcept {
        boost::asio::post(m_threadPool, [&]() noexcept {
            AddSubDirectoriesToQueue(m_options.path, *m_fileTree->GetRoot());
        });

        m_threadPool.join();
    });

    const auto& log = spdlog::get(Constants::Logging::DefaultLog);
    log->info(
        "Scanned Drive in: {:L} {}", stopwatch.GetElapsedTime().count(),
        stopwatch.GetUnitsAsString());

    Scanner::ComputeDirectorySizes(*m_fileTree);
    PruneEmptyFilesAndDirectories(*m_fileTree);

    emit Finished(m_fileTree);
}
