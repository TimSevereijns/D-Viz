#ifndef SCANNINGPROGRESS_HPP
#define SCANNINGPROGRESS_HPP

#include <atomic>
#include <chrono>
#include <cstdint>

/**
 * @brief Various pieces of metadata to track file system scan progress.
 */
class ScanningProgress
{
  public:
    /**
     * @brief Resets the scanning progress metadata.
     */
    void Reset() noexcept
    {
        filesScanned.store(0);
        directoriesScanned.store(0);
        bytesProcessed.store(0);

        m_startTime = std::chrono::steady_clock::now();
    }

    /**
     * @returns The elapsed seconds since the start of the scan.
     */
    std::chrono::seconds GetElapsedSeconds() const noexcept
    {
        return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - m_startTime);
    }

    std::atomic<std::uintmax_t> filesScanned;
    std::atomic<std::uintmax_t> directoriesScanned;
    std::atomic<std::uintmax_t> bytesProcessed;

  private:
    std::chrono::steady_clock::time_point m_startTime;
};

#endif // SCANNINGPROGRESS_HPP
