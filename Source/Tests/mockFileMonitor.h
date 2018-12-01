#ifndef MOCKFILEMONITOR_H
#define MOCKFILEMONITOR_H

#include <Visualizations/fileMonitorBase.h>
#include <Visualizations/fileChangeNotification.hpp>

#include <experimental/filesystem>
#include <functional>
#include <thread>

/**
 * @brief The MockFileMonitor class
 */
class MockFileMonitor : public FileMonitorBase
{
public:

   MockFileMonitor(std::function<FileChangeNotification ()> notificationGenerator)
      : m_notificationGenerator{ std::move(notificationGenerator) }
   {
   }

   ~MockFileMonitor() noexcept override
   {
      if (m_workerThread.joinable())
      {
         m_workerThread.join();
      }
   }

   void Start(
      const std::experimental::filesystem::path& path,
      const std::function<void (FileChangeNotification&&)>& callback) override
   {
      m_pathToMonitor = path;
      m_callback = callback;
      m_workerThread = std::thread{ [&]{ SendFakeNotification(); } };
      m_isActive = true;
   }

   void Stop() override
   {
      m_isActive = false;
   }

   bool IsActive() const override
   {
      return m_isActive;
   }

private:

   void SendFakeNotification() const noexcept
   {
      auto notification = m_notificationGenerator();
      m_callback(std::move(notification));
   }

   std::function<FileChangeNotification ()> m_notificationGenerator;

   std::function<void (FileChangeNotification&&)> m_callback;

   std::thread m_workerThread;

   std::experimental::filesystem::path m_pathToMonitor;

   bool m_isActive{ false };
};

#endif // MOCKFILEMONITOR_H
