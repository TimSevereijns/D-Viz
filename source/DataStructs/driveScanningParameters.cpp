#include "driveScanningParameters.h"

DriveScanningParameters::DriveScanningParameters(
   const std::wstring& startingPath,
   ProgressCallback progressCallback,
   ScanCompleteCallback completionCallback)
   :
   path{ startingPath },
   onProgressUpdateCallback{ progressCallback },
   onScanCompletedCallback{ completionCallback }
{
}
