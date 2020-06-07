#include "Model/Scanner/scanningParameters.h"

#include <utility>

ScanningParameters::ScanningParameters(
    std::filesystem::path startingPath, ProgressCallback progressCallback,
    ScanCompleteCallback completionCallback)
    : path{ std::move(startingPath) },
      onProgressUpdateCallback{ std::move(progressCallback) },
      onScanCompletedCallback{ std::move(completionCallback) }
{
}
