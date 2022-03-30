#ifndef SCANNINGOPTIONS_H
#define SCANNINGOPTIONS_H

#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>

class ScanningProgress;
template <typename T> class Tree;
class VizBlock;

/**
 * @brief Wrapper around all of the options needed to scan a directories, as well as to track
 * progress.
 */
class ScanningOptions
{
  public:
    using ProgressCallback = std::function<void(const ScanningProgress&)>;

    using ScanCompleteCallback =
        std::function<void(const ScanningProgress&, std::shared_ptr<Tree<VizBlock>> fileTree)>;

    ScanningOptions() = default;

    ScanningOptions(
        std::filesystem::path startingPath, ProgressCallback progressCallback,
        ScanCompleteCallback completionCallback);

    std::filesystem::path path;

    ProgressCallback onProgressUpdateCallback;
    ScanCompleteCallback onScanCompletedCallback;
};

#endif // SCANNINGOPTIONS_H
