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

#include "../DataStructs/block.h"
#include "../DataStructs/fileInfo.h"
#include "../DataStructs/vizNode.h"

#include "../Visualizations/visualization.h"

#include "../tree.h"

class ScanningWorker : public QObject
{
   Q_OBJECT

   public:
      static const std::uintmax_t SIZE_UNDEFINED;

      explicit ScanningWorker(QObject* parent = nullptr);

   public slots:
      void Start();

   signals:
      void Error(std::wstring message);
      void Finished(const std::uintmax_t filesScanned);
      void ProgressUpdate(const std::uintmax_t filesScanned);

   private:
      /**
       * Max path length in Windows is 260 characters, so if that includes slashes, then the maximum
       * depth of a directory or file is no more than 130, or so. Given that the default stack size
       * in MSVC is 1MB, and I only pass in references, this recursive version may be fine---maybe!
       */
      void ScanRecursively(const boost::filesystem::path& path, TreeNode<VizNode>& fileNode);

      void ComputeDirectorySizes();

      std::shared_ptr<Tree<VizNode>> m_fileTree;

      boost::filesystem::path m_path;

      std::uintmax_t m_filesScanned;

      std::chrono::duration<double> m_scanningTime;
};

#endif // SCANNINGTHREAD_H
