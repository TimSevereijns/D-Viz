#include "driveScanningParameters.h"

DriveScanningParameters::DriveScanningParameters()
   : onProgressUpdateCallback(ProgressCallback{}),
     onScanCompletedCallback(ScanCompleteCallback{}),
     path(L"")
{
}
