#include "diskScanner.h"

#include "Visualizations/visualization.h"

#include <algorithm>
#include <codecvt>
#include <chrono>
#include <iostream>
#include <locale>
#include <numeric>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

const std::uintmax_t DiskScanner::SIZE_UNDEFINED = 0;

namespace
{
   /**
    * Traverses the file tree from beginning to end, accumulating the file sizes (in bytes) of all
    * regular (non-directory, or symbolic) files.
    *
    * @param[in] fileTree           The tree to be traversed.
    * @returns The size in bytes of all files in the tree.
    */
   std::uintmax_t ComputeTopLevelDirectorySizeInBytesViaTraversal(const Tree<VizNode>& fileTree)
   {
      const std::uintmax_t treeSize = std::accumulate(fileTree.beginLeaf(), fileTree.endLeaf(),
         std::uintmax_t{0}, [] (const std::uintmax_t result, const TreeNode<VizNode>& node)
      {
         const FileInfo fileInfo = node.GetData().m_file;
         if (fileInfo.m_type == FILE_TYPE::REGULAR)
         {
            return result + fileInfo.m_size;
         }

         return result;
      });

      return treeSize;
   }

   /**
    * @brief WideStringToNarrowString converts the given wide string to a narrow string.
    * 
    * @param[in] wide                  The wide string to be converted.
    * @returns a narrow string.
    */
   std::string WideStringToNarrowString(const std::wstring& wide)
   {
      typedef std::codecvt_utf8<wchar_t> convertType;
      std::wstring_convert<convertType, wchar_t> converter;
      return converter.to_bytes(wide);
   }

   /**
    * @brief NarrowStringToWideString converts the given string to a wide string.
    * 
    * @param[in] narrow                The string to convert.
    * @returns a wide string
    */
   std::wstring NarrowStringToWideString(const std::string& narrow)
   {
       typedef std::codecvt_utf8<wchar_t> convertType;
       std::wstring_convert<convertType, wchar_t> converter;
       return converter.from_bytes(narrow);
   }

   /**
    * @brief SerializeRecursively is a helper function to recursively serialize the filesystem tree
    * into the specified JSON array.
    * 
    * @param[in/out] jsonObject           The array into which the tree is to be serialized.
    * @param[in] fileNode                 The tree node to be serialized.
    */
   void SerializeRecursively(QJsonArray& jsonArray, TreeNode<VizNode>* fileNode)
   {
      if (!fileNode)
      {
         return;
      }

      FileInfo& fileInfo = fileNode->GetData().m_file;

      if (fileInfo.m_type == FILE_TYPE::REGULAR)
      {
         QJsonObject file;
         file["name"] = QString::fromStdWString(fileInfo.m_name);
         file["size"] = QString::fromStdWString(std::to_wstring(fileInfo.m_size));

         jsonArray.append(file);
      }
      else if (fileInfo.m_type == FILE_TYPE::DIRECTORY)
      {
         QJsonObject directory;
         QJsonArray content;

         fileNode = fileNode->GetFirstChild().get();
         while (fileNode)
         {
            SerializeRecursively(content, fileNode);

            fileNode = fileNode->GetNextSibling().get();
         }

         directory[QString::fromStdWString(fileInfo.m_name)] = content;
         jsonArray.append(directory);
      }
   }
}

DiskScanner::DiskScanner()
{
}

DiskScanner::DiskScanner(const std::wstring& rawPath)
   : m_fileTree(nullptr),
     m_filesScanned(0),
     m_scanningTime(std::chrono::duration<double>(0))
{
   boost::filesystem::path path{rawPath};
   path.make_preferred();

   const bool isPathValid = boost::filesystem::exists(path);
   if (!isPathValid)
   {
      throw std::invalid_argument("The provided path does not seem to exist!");
   }

   m_path = path;
}

DiskScanner::~DiskScanner()
{
}

void DiskScanner::StartScanning(std::atomic<std::pair<std::uintmax_t, bool>>* progress)
{
   assert(boost::filesystem::is_directory(m_path));

   const Block rootBlock
   {
      QVector3D(0, 0, 0),
      Visualization::ROOT_BLOCK_WIDTH,
      Visualization::BLOCK_HEIGHT,
      Visualization::ROOT_BLOCK_DEPTH
   };

   // Dummy root node:
   FileInfo fileInfo{L"Dummy Root Node", DiskScanner::SIZE_UNDEFINED, FILE_TYPE::DIRECTORY};
   VizNode rootNode{fileInfo, rootBlock};

   m_fileTree = std::make_unique<Tree<VizNode>>(Tree<VizNode>(rootNode));

   try
   {
      const auto start = std::chrono::high_resolution_clock::now();
      ScanRecursively(m_path, *m_fileTree->GetHead(), progress);
      const auto end = std::chrono::high_resolution_clock::now();

      m_scanningTime = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);

      progress->store(std::make_pair(m_filesScanned, /*isScanningDone =*/ true));

      ComputeDirectorySizes();
   }
   catch (const boost::filesystem::filesystem_error& exception)
   {
      std::cout << exception.what() << std::endl;
   }
}

void DiskScanner::ScanRecursively(const boost::filesystem::path& path, TreeNode<VizNode>& treeNode,
   std::atomic<std::pair<std::uintmax_t, bool>>* progress)
{
   if (boost::filesystem::is_symlink(path))
   {
      return;
   }

   progress->store(std::make_pair(m_filesScanned, /*isScanningDone =*/ false));

   if (boost::filesystem::is_regular_file(path) && boost::filesystem::file_size(path) > 0)
   {
      FileInfo fileInfo(path.filename().wstring(), boost::filesystem::file_size(path),
         FILE_TYPE::REGULAR);
      treeNode.AppendChild(VizNode(fileInfo));

      ++m_filesScanned;
   }
   else if (boost::filesystem::is_directory(path) && !boost::filesystem::is_empty(path))
   {
      FileInfo directoryInfo(path.filename().wstring(), DiskScanner::SIZE_UNDEFINED,
         FILE_TYPE::DIRECTORY);
      treeNode.AppendChild(VizNode(directoryInfo));

      ++m_filesScanned;

      for (auto itr = boost::filesystem::directory_iterator(path);
           itr != boost::filesystem::directory_iterator();
           ++itr)
      {
         const boost::filesystem::path nextPath = itr->path();
         ScanRecursively(nextPath, *treeNode.GetLastChild(), progress);
      }
   }
}

void DiskScanner::ScanInNewThread(std::atomic<std::pair<std::uintmax_t, bool>>* progress)
{
   m_scanningThread = std::thread(&DiskScanner::StartScanning, this, progress);
}

void DiskScanner::JoinScanningThread()
{
   m_scanningThread.join();
}

void DiskScanner::ComputeDirectorySizes()
{
   assert(m_fileTree);

   std::for_each(std::begin(*m_fileTree), std::end(*m_fileTree),
      [] (const TreeNode<VizNode>& node)
   {
      const FileInfo fileInfo = node.GetData().m_file;

      std::shared_ptr<TreeNode<VizNode>>& parent = node.GetParent();
      if (parent)
      {
         FileInfo& parentInfo = parent->GetData().m_file;
         if (parentInfo.m_type == FILE_TYPE::DIRECTORY)
         {
            parentInfo.m_size += fileInfo.m_size;
         }
      }
   });
}

std::uintmax_t DiskScanner::GetNumberOfFilesScanned()
{
   return m_filesScanned;
}

Tree<VizNode>& DiskScanner::GetFileTree() const
{
   return *m_fileTree.get();
}

void DiskScanner::PrintTree() const
{
   std::cout << "=============" << std::endl;
   std::cout << "  The Tree!  " << std::endl;
   std::cout << "=============" << std::endl;

   Tree<VizNode>::Print(*m_fileTree->GetHead(),
      [] (const VizNode& data) { return data.m_file.m_name; } );
}

void DiskScanner::PrintTreeMetadata() const
{
   const std::uintmax_t sizeInBytes = ComputeTopLevelDirectorySizeInBytesViaTraversal(*m_fileTree);
   const double sizeInMegabytes = DiskScanner::ConvertBytesToMegaBytes(sizeInBytes);

   const unsigned int nodeCount = Tree<VizNode>::Size(*m_fileTree->GetHead());

   const auto startTime = std::chrono::high_resolution_clock::now();
   const auto fileCount = std::count_if(std::begin(*m_fileTree), std::end(*m_fileTree),
    [] (const TreeNode<VizNode>& node)
   {
      return node.GetData().m_file.m_type == FILE_TYPE::REGULAR;
   });
   const auto endTime = std::chrono::high_resolution_clock::now();

   std::chrono::duration<double> traversalTime =
      std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime);

   std::cout.imbue(std::locale(""));

   std::cout << "=============" << std::endl;
   std::cout << "Tree Metadata" << std::endl;
   std::cout << "=============" << std::endl;

   std::cout << "File Size (Logical), Computed via Traversal:" << std::endl;
   std::cout << sizeInMegabytes << " MB (" << sizeInBytes << " bytes)" << std::endl;

   std::cout << "Top Level Directory Size, via Single Look-up:" << std::endl;
   std::cout << m_fileTree->GetHead()->GetData().m_file.m_size << " bytes" << std::endl;

   std::cout << "Total Node Count:" << std::endl;
   std::cout << nodeCount << std::endl;

   std::cout << "File Count:" << std::endl;
   std::cout << fileCount << std::endl;

   std::cout << "Folder Count:" << std::endl;
   std::cout << nodeCount - 1 - fileCount << std::endl;

   std::cout << "Scanning Time (in seconds):" << std::endl;
   std::cout << m_scanningTime.count() << std::endl;

   std::cout << "Tree Traversal Time (in seconds):" << std::endl;
   std::cout << traversalTime.count() << std::endl;
}

void DiskScanner::ToJSON(QJsonObject& json)
{
   if (!m_fileTree)
   {
      return;
   }

   std::shared_ptr<TreeNode<VizNode>> firstNode = m_fileTree->GetHead();

   QJsonArray fileTree;
   SerializeRecursively(fileTree, firstNode.get());

   json["root"] = fileTree;
}

double DiskScanner::ConvertBytesToMegaBytes(const std::uintmax_t bytes)
{
   const double oneMegabyte = std::pow(2, 20);
   return bytes / oneMegabyte;
}

double DiskScanner::ConvertBytesToGigaBytes(const std::uintmax_t bytes)
{
   const double oneGigabyte = std::pow(2, 30);
   return bytes / oneGigabyte;
}
