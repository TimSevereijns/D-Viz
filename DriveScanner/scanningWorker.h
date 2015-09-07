#ifndef SCANNINGTHREAD_H
#define SCANNINGTHREAD_H

#include <boost/filesystem.hpp>

#include <QObject>
#include <QVector>
#include <QVector3D>

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

#include "../tree.h"

class ScanningWorker : public QObject
{
   Q_OBJECT

   public:
      explicit ScanningWorker(QObject *parent = 0);
      ~ScanningWorker();

   public slots:
      void Start();

   signals:
      void Finished(const std::uintmax_t filesScanned);
      void Error(std::wstring message);
};

#endif // SCANNINGTHREAD_H
