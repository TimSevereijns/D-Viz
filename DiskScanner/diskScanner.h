#ifndef DISKSCANNER_H
#define DISKSCANNER_H

#include <QObject>
#include <QThread>

#include <cstdint>

class DiskScanner : public QObject
{
   Q_OBJECT

   public:
      explicit DiskScanner(QObject *parent = 0);
      ~DiskScanner();

      void StartScanning();

   public slots:
      //void HandleProgressUpdates(const std::uintmax_t filesScanned);

   private:
      QThread m_workerThread;
};

#endif // DISKSCANNER_H
