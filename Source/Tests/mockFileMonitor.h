#ifndef MOCKFILEMONITOR_H
#define MOCKFILEMONITOR_H

#include <Scanner/Monitor/fileChangeNotification.hpp>
#include <Scanner/Monitor/fileMonitorBase.h>

#include <boost/optional.hpp>

#include <atomic>
#include <filesystem>
#include <functional>
#include <thread>

/**
 * @brief The MockFileMonitor class
 */
class MockFileMonitor : public FileMonitorBase
{
  public:
    MockFileMonitor(std::function<boost::optional<FileEvent>()> notificationGenerator)
        : m_notificationGenerator{ std::move(notificationGenerator) }
    {
    }

    ~MockFileMonitor() noexcept override
    {
        Stop();

        if (m_workerThread.joinable()) {
            m_workerThread.join();
        }
    }

    void Start(
        const std::filesystem::path& path,
        const std::function<void(FileEvent&&)>& onNotificationCallback) override
    {
        m_pathToMonitor = path;
        m_onNotificationCallback = onNotificationCallback;

        m_isActive.store(true);
        m_workerThread = std::thread{ [&] { SendFakeNotification(); } };
    }

    void Stop() override
    {
        m_isActive.store(false);
    }

    bool IsActive() const override
    {
        return m_isActive.load();
    }

  private:
    void SendFakeNotification() const noexcept
    {
        while (m_isActive) {
            auto optionalNotification = m_notificationGenerator();

            if (optionalNotification) {
                m_onNotificationCallback(std::move(*optionalNotification));
            }
        }
    }

    std::function<boost::optional<FileEvent>()> m_notificationGenerator;

    std::function<void(FileEvent&&)> m_onNotificationCallback;

    std::thread m_workerThread;

    std::filesystem::path m_pathToMonitor;

    std::atomic_bool m_isActive{ false };
};

#endif // MOCKFILEMONITOR_H
