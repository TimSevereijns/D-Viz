#ifndef DISKSCANNER_H
#define DISKSCANNER_H

#include <QObject>
#include <QThread>

#include <cstdint>

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
      void HandleProgressUpdates(const std::uintmax_t filesScanned);

   private:
      QThread m_workerThread;
};

#endif // DISKSCANNER_H
