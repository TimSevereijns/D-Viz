#include "driveScanningParameters.h"

#include <utility>

DriveScanningParameters::DriveScanningParameters(
   std::experimental::filesystem::path startingPath,
   ProgressCallback progressCallback,
   ScanCompleteCallback completionCallback)
   :
   path{ std::move(startingPath)},
   onProgressUpdateCallback{ std::move(progressCallback) },
   onScanCompletedCallback{ std::move(completionCallback) }
{
}
