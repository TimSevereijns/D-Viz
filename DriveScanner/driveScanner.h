#ifndef DRIVESCANNER_H
#define DRIVESCANNER_H

#include <QObject>
#include <QThread>

#include <cstdint>
#include <memory>
#include <string>

#include "scanningWorker.h"

/**
 * @brief The DriveScannerParameters struct
 */
struct DriveScannerParameters
{
   using ProgressCallback = std::function<void (const std::uintmax_t filesScanned)>;
   using ScanCompleteCallback = std::function<void (const std::uintmax_t filesScanned)>;

   ProgressCallback onProgressUpdateCallback;
   ScanCompleteCallback onScanCompletedCallback;

   std::wstring path;

   DriveScannerParameters()
      : onProgressUpdateCallback(ProgressCallback()),
        onScanCompletedCallback(ScanCompleteCallback()),
        path(L"")
   {
   }
};

/**
 * @brief The DriveScanner class
 */
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

      void HandleMessageBox(const QString& message);

   private:
      DriveScannerParameters m_scanningParameters;

      std::shared_ptr<Tree<VizNode>> m_theTree;
};

#endif // DRIVESCANNER_H
