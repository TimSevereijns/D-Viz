#ifndef DRIVESCANNER_H
#define DRIVESCANNER_H

#include <QObject>
#include <QThread>

#include <cstdint>
#include <memory>

#include "scanningWorker.h"

/**
 * @brief The DiskScanner class
 *
 * Dev Resources:
 *
 *    http://doc.qt.io/qt-5.4/qthread.html
 *    https://wiki.qt.io/QThreads_general_usage
 *    https://mayaposch.wordpress.com/2011/11/01/how-to-really-truly-use-qthreads-the-full-explanation/
 */

class DriveScanner : public QObject
{
   Q_OBJECT

   public:
      explicit DriveScanner(QObject* parent = nullptr);

      void StartScanning();

   public slots:
      void HandleCompletion(const std::uintmax_t filesScanned);
      void HandleProgressUpdates(const std::uintmax_t filesScanned);
      void HandleErrors(const std::wstring message);

   private:
      std::unique_ptr<QThread> m_thread;
      std::unique_ptr<ScanningWorker> m_worker;
};

#endif // DRIVESCANNER_H
