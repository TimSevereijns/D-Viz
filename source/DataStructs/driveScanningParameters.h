#ifndef DRIVESCANNINGPARAMETERS_H
#define DRIVESCANNINGPARAMETERS_H

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

template<typename T>
class Tree;

struct ScanningProgress;
struct VizFile;

/**
 * @brief Wrapper around all of the parameters needed to scan a directories, as well as to track
 * progress.
 */
struct DriveScanningParameters
{
   using ProgressCallback = std::function<void (const ScanningProgress&)>;

   using ScanCompleteCallback =
      std::function<void (const ScanningProgress&, std::shared_ptr<Tree<VizFile>> fileTree)>;

   std::wstring path{ };

   ProgressCallback onProgressUpdateCallback{ };
   ScanCompleteCallback onScanCompletedCallback{ };

   DriveScanningParameters() = default;

   DriveScanningParameters(
      const std::wstring& startingPath,
      ProgressCallback progressCallback,
      ScanCompleteCallback completionCallback);
};

#endif // DRIVESCANNINGPARAMETERS_H
