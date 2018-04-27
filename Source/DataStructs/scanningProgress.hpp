#ifndef SCANNINGPROGRESS_HPP
#define SCANNINGPROGRESS_HPP

#include <atomic>
#include <cstdint>

struct ScanningProgress
{
   void Reset()
   {
      filesScanned.store(0);
      directoriesScanned.store(0);
      bytesProcessed.store(0);
   }

   std::atomic<std::uintmax_t> filesScanned;
   std::atomic<std::uintmax_t> directoriesScanned;
   std::atomic<std::uintmax_t> bytesProcessed;
};

#endif // SCANNINGPROGRESS_HPP
