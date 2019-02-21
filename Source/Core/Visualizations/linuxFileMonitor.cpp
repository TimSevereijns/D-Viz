#include "linuxFileMonitor.h"

#include "../constants.h"

#include <gsl/gsl_assert>
#include <spdlog/spdlog.h>

#ifdef Q_OS_LINUX

LinuxFileMonitor::~LinuxFileMonitor() noexcept
{
    if (m_isActive) {
        Stop();
    }
}

void LinuxFileMonitor::Start(
    const std::experimental::filesystem::path& path,
    const std::function<void(FileChangeNotification&&)>& onNotificationCallback)
{
    m_pathToWatch = path.native();

    auto handleNotification = [&](const inotify::Notification& notification) noexcept
    {
        using namespace inotify;

        // @todo inotify::Events are not easily convertible to strings, but the stream output
        // operator
        // does exist.
        std::stringstream stream;
        stream << "Event " << notification.event << " on " << notification.path << " at "
               << notification.time.time_since_epoch().count() << " was triggered.";

        const auto& log = spdlog::get(Constants::Logging::FILESYSTEM_LOG);
        log->info(stream.str());

        ProcessNotification(notification, onNotificationCallback);
    };

    auto handleUnexpectedNotification = [](const inotify::Notification& notification) noexcept
    {
        using namespace inotify;

        // @todo inotify::Events are not easily convertible to strings, but the stream output
        // operator
        // does exist.
        std::stringstream stream;
        stream << "Event " << notification.event << " on " << notification.path << " at "
               << notification.time.time_since_epoch().count()
               << " was triggered, but not expected.";

        const auto& log = spdlog::get(Constants::Logging::FILESYSTEM_LOG);
        log->error(stream.str());
    };

    // Set the events to be notified for
    auto events = { inotify::Event::create, inotify::Event::modify, inotify::Event::remove,
                    inotify::Event::move };

    try {
        // The notifier is configured to watch the parsed path for the defined events.
        m_notifier = inotify::BuildNotifier()
                         .watchPathRecursively(m_pathToWatch)
                         .onEvents(events, handleNotification)
                         .onUnexpectedEvent(handleUnexpectedNotification);
    } catch (const std::exception& exception) {
        const auto& log = spdlog::get(Constants::Logging::FILESYSTEM_LOG);
        log->error(exception.what());
    }

    m_monitoringThread = std::thread
    {
        [&]() noexcept
        {
            m_notifier.run();
        }
    }

    m_isActive = true;
}

void LinuxFileMonitor::Stop()
{
    m_isActive = false;

    m_notifier.stop();

    if (m_monitoringThread.joinable()) {
        m_monitoringThread.join();
    }
}

bool LinuxFileMonitor::IsActive() const
{
    return m_isActive;
}

bool LinuxFileMonitor::ProcessNotification(
    const inotify::Notification& notification,
    const std::function<void(FileChangeNotification&&)>& callback) const
{
    const std::experimental::filesystem::path path =
        boost::filesystem::relative(notification.path, m_pathToWatch).string();

    switch (notification.event) {
        case inotify::Event::create: {
            callback(FileChangeNotification{ path, FileModification::CREATED });
            break;
        }
        case inotify::Event::remove: {
            callback(FileChangeNotification{ path, FileModification::DELETED });
            break;
        }
        case inotify::Event::modify: {
            callback(FileChangeNotification{ path, FileModification::TOUCHED });
            break;
        }
        case inotify::Event::moved_from: {
            // @todo
            break;
        }
        case inotify::Event::moved_to: {
            // @todo
            break;
        }
        default:
            break;
    }

    return true;
}

#endif // Q_OS_LINUX
