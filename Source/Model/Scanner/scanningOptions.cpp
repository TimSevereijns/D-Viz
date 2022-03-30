#include "Model/Scanner/scanningOptions.h"

#include <utility>

ScanningOptions::ScanningOptions(
    std::filesystem::path startingPath, ProgressCallback progressCallback,
    ScanCompleteCallback completionCallback)
    : path{ std::move(startingPath) },
      onProgressUpdateCallback{ std::move(progressCallback) },
      onScanCompletedCallback{ std::move(completionCallback) }
{
}
