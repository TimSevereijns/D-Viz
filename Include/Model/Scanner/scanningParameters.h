#ifndef SCANNINGPARAMETERS_H
#define SCANNINGPARAMETERS_H

#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>

class ScanningProgress;
template <typename T> class Tree;
class VizBlock;

/**
 * @brief Wrapper around all of the parameters needed to scan a directories, as well as to track
 * progress.
 */
class ScanningParameters
{
  public:
    using ProgressCallback = std::function<void(const ScanningProgress&)>;

    using ScanCompleteCallback =
        std::function<void(const ScanningProgress&, std::shared_ptr<Tree<VizBlock>> fileTree)>;

    ScanningParameters() = default;

    ScanningParameters(
        std::filesystem::path startingPath, ProgressCallback progressCallback,
        ScanCompleteCallback completionCallback);

    std::filesystem::path path;

    ProgressCallback onProgressUpdateCallback;
    ScanCompleteCallback onScanCompletedCallback;
};

#endif // SCANNINGPARAMETERS_H
