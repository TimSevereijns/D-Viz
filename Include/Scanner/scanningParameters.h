#ifndef SCANNINGPARAMETERS_H
#define SCANNINGPARAMETERS_H

#include <cstdint>
#include <filesystem>
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

    std::filesystem::path path{};

    ProgressCallback onProgressUpdateCallback{};
    ScanCompleteCallback onScanCompletedCallback{};

    ScanningParameters() = default;

    ScanningParameters(
        std::filesystem::path startingPath, ProgressCallback progressCallback,
        ScanCompleteCallback completionCallback);
};

#endif // SCANNINGPARAMETERS_H
