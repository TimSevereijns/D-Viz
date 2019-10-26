#ifndef SCANNINGWORKER_H
#define SCANNINGWORKER_H

#include <QObject>
#include <QVector3D>
#include <QVector>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>

#ifdef Q_OS_WIN
#pragma warning(push)
#pragma warning(disable : 4996)
#endif // Q_OS_WIN

#include <boost/asio/thread_pool.hpp>

#ifdef Q_OS_WIN
#pragma warning(pop)
#endif // Q_OS_WIN

#include <Tree/Tree.hpp>

#include "Scanner/fileInfo.h"
#include "Scanner/scanningParameters.h"
#include "Scanner/scanningProgress.hpp"
#include "Visualizations/block.h"
#include "Visualizations/visualization.h"
#include "Visualizations/vizBlock.h"

/**
 * @brief The NodeAndPath struct
 */
struct NodeAndPath
{
    std::unique_ptr<Tree<VizBlock>::Node> node;
    std::filesystem::path path;

    NodeAndPath(decltype(node) node, decltype(path) path)
        : node{ std::move(node) }, path{ std::move(path) }
    {
    }

    NodeAndPath() = default;
};

/**
 * @brief The ScanningWorker class
 */
class ScanningWorker final : public QObject
{
    Q_OBJECT

  public:
    static constexpr std::uintmax_t SIZE_UNDEFINED{ 0 };

    ScanningWorker(const ScanningParameters& parameters, ScanningProgress& progress);

  public slots:

    /**
     * @brief Kicks off the drive scanning process.
     *
     * As part of the scanning process, the ProgressUpdate signal will be fired to signal progress
     * updates, and the Finish signal will be fired once the scanning process completes
     * successfully.
     *
     * @see Finished
     * @see ProgressUpdate
     */
    void Start();

  signals:

    /**
     * @brief Signals that the drive scanning has finished.
     *
     * @param[in] fileTree        A pointer to the final tree representing the scanned drive.
     */
    void Finished(const std::shared_ptr<Tree<VizBlock>>& fileTree);

    /**
     * @brief Signals drive scanning progress updates.
     */
    void ProgressUpdate();

    /**
     * @brief Allows for cross-thread signaling to show the user a standard Qt message box.
     *
     * @param[in] message         The message to be shown in the message box.
     */
    void ShowMessageBox(const QString& message);

  private:
    /**
     * @brief Helper function to process a single file.
     *
     * @note This function assumes the path is valid and accessible.
     *
     * @param[in] path            The location on disk to scan.
     * @param[in] fileNode        The TreeNode in Tree to append newly discoved files to.
     */
    void ProcessFile(const std::filesystem::path& path, Tree<VizBlock>::Node& node) noexcept;

    /**
     * @brief Performs a recursive depth-first exploration of the file system.
     *
     * @param[in] path            The location on disk to scan.
     * @param[in] fileNode        The TreeNode in Tree to append newly discoved files to.
     */
    void ProcessDirectory(const std::filesystem::path& path, Tree<VizBlock>::Node& node) noexcept;

    /**
     * @brief Helper function to facilitate exception-free iteration over a directory.
     *
     * @param[in] itr             Reference to the iterator to iterate over.
     * @param[in] treeNode        The TreeNode to append the contents of the directory to.
     */
    void AddSubDirectoriesToQueue(
        const std::filesystem::path& path, Tree<VizBlock>::Node& node) noexcept;

    ScanningParameters m_parameters;

    ScanningProgress& m_progress;

    std::shared_ptr<Tree<VizBlock>> m_fileTree{ nullptr };

    mutable std::mutex m_mutex;

    boost::asio::thread_pool m_threadPool{ 4 };
};

#endif // SCANNINGWORKER_H
