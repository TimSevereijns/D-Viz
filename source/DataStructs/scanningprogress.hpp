#ifndef SCANNINGPROGRESS_HPP
#define SCANNINGPROGRESS_HPP

#include <atomic>
#include <cstdint>

struct ScanningProgress
{
   void Reset()
   {
      filesScanned.store(0);
      numberOfBytesProcessed.store(0);
   }

   std::atomic<std::uintmax_t> filesScanned;
   std::atomic<std::uintmax_t> numberOfBytesProcessed;
};

#endif // SCANNINGPROGRESS_HPP
