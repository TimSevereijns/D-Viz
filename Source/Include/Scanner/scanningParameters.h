#ifndef DRIVESCANNINGPARAMETERS_H
#define DRIVESCANNINGPARAMETERS_H

#include <cstdint>
#include <experimental/filesystem>
#include <functional>
#include <memory>
#include <string>

template <typename T> class Tree;

struct ScanningProgress;
struct VizBlock;

/**
 * @brief Wrapper around all of the parameters needed to scan a directories, as well as to track
 * progress.
 */
struct ScanningParameters
{
    using ProgressCallback = std::function<void(const ScanningProgress&)>;

    using ScanCompleteCallback =
        std::function<void(const ScanningProgress&, std::shared_ptr<Tree<VizBlock>> fileTree)>;

    std::experimental::filesystem::path path{};

    ProgressCallback onProgressUpdateCallback{};
    ScanCompleteCallback onScanCompletedCallback{};

    ScanningParameters() = default;

    ScanningParameters(
        std::experimental::filesystem::path startingPath, ProgressCallback progressCallback,
        ScanCompleteCallback completionCallback);
};

#endif // DRIVESCANNINGPARAMETERS_H
