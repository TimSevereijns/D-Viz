#include "driveScanningParameters.h"

DriveScanningParameters::DriveScanningParameters()
   : onProgressUpdateCallback(ProgressCallback{}),
     onScanCompletedCallback(ScanCompleteCallback{}),
     path(L"")
{
}

DriveScanningParameters::DriveScanningParameters(
   const std::wstring& startingPath,
   ProgressCallback progressCallback,
   ScanCompleteCallback completionCallback)
   : path(startingPath),
     onProgressUpdateCallback(progressCallback),
     onScanCompletedCallback(completionCallback)
{
}
