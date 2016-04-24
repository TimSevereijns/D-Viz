#ifndef DRIVESCANNINGPARAMETERS_H
#define DRIVESCANNINGPARAMETERS_H

#include <cstdint>
#include <string>
#include <functional>

namespace std
{
   template<typename T> class shared_ptr;
}

template<typename T>
class Tree;

struct VizNode;

/**
 * @brief The DriveScannerParameters struct
 */
struct DriveScanningParameters
{
   using ProgressCallback = std::function<void (const std::uintmax_t filesScanned)>;
   using ScanCompleteCallback = std::function<void (const std::uintmax_t filesScanned,
      std::shared_ptr<Tree<VizNode>> fileTree)>;

   ProgressCallback onProgressUpdateCallback{ };
   ScanCompleteCallback onScanCompletedCallback{ };

   std::wstring path{ };

   DriveScanningParameters() = default;

   DriveScanningParameters(
      const std::wstring& startingPath,
      ProgressCallback progressCallback,
      ScanCompleteCallback completionCallback);
};

#endif // DRIVESCANNINGPARAMETERS_H
