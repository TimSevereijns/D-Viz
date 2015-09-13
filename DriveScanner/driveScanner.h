#ifndef DRIVESCANNER_H
#define DRIVESCANNER_H

#include <QObject>
#include <QThread>

#include <cstdint>
#include <memory>
#include <string>

#include "scanningWorker.h"

/**
 * @brief The DiskScanner class
 *
 * Dev Resources:
 *
 *    http://doc.qt.io/qt-5.4/qthread.html
 *    https://wiki.qt.io/QThreads_general_usage
 *    https://mayaposch.wordpress.com/2011/11/01/
 *       how-to-really-truly-use-qthreads-the-full-explanation/
 */

struct DriveScannerParameters
{
   std::function<void (const std::wstring message)> m_onErrorCallback;
   std::function<void (const std::uintmax_t filesScanned)> m_onProgressUpdateCallback;
   std::function<void (const std::uintmax_t filesScanned)> m_onScanCompletedCallback;

   std::wstring m_path;

   DriveScannerParameters()
      : m_onErrorCallback(std::function<void (const std::wstring)>()),
        m_onProgressUpdateCallback(std::function<void (const std::uintmax_t)>()),
        m_onScanCompletedCallback(std::function<void (const std::uintmax_t)>()),
        m_path(L"")
   {
   }
};

class DriveScanner : public QObject
{
   Q_OBJECT

   public:
      explicit DriveScanner();
      ~DriveScanner();

      void SetParameters(const DriveScannerParameters& parameters);
      void StartScanning();

      std::shared_ptr<Tree<VizNode>> GetTree() const;

   public slots:
      void HandleCompletion(const std::uintmax_t filesScanned);
      void HandleProgressUpdates(const std::uintmax_t filesScanned);
      void HandleErrors(const std::wstring message);

   private:
      DriveScannerParameters m_scanningParameters;

      //std::unique_ptr<QThread> m_thread;
      //std::unique_ptr<ScanningWorker> m_worker;

      std::shared_ptr<Tree<VizNode>> m_theTree;
};

#endif // DRIVESCANNER_H
