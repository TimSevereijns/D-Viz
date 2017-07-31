#ifndef DRIVESCANNINGPARAMETERS_H
#define DRIVESCANNINGPARAMETERS_H

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

template<typename T>
class Tree;

struct VizFile;

/**
 * @brief The DriveScannerParameters struct
 */
struct DriveScanningParameters
{
   using ProgressCallback = std::function<void (const std::uintmax_t filesScanned,
      const std::uintmax_t numberOfBytesProcessed)>;

   using ScanCompleteCallback = std::function<void (const std::uintmax_t filesScanned,
      const std::uintmax_t numberOfBytesProcessed, std::shared_ptr<Tree<VizFile>> fileTree)>;

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
