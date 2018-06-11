#include "driveScanningParameters.h"

DriveScanningParameters::DriveScanningParameters(
   const std::experimental::filesystem::path& startingPath,
   ProgressCallback progressCallback,
   ScanCompleteCallback completionCallback)
   :
   path{ startingPath },
   onProgressUpdateCallback{ progressCallback },
   onScanCompletedCallback{ completionCallback }
{
}
