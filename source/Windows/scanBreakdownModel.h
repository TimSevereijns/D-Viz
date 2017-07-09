#ifndef SCANBREAKDOWNENTRY_H
#define SCANBREAKDOWNENTRY_H

#include <QAbstractTableModel>

#include "../controller.h"
#include "../DataStructs/vizFile.h"

#include <Tree/Tree.hpp>

#include <mutex>
#include <sstream>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace
{
   std::once_flag stringStreamSetupFlag;

   template<
      typename Type,
      typename = std::enable_if<std::is_arithmetic<Type>::value>
   >
   static auto ConvertToFormattedSizeString(const Type& number)
   {
      static std::wstringstream stringStream;

      std::call_once(stringStreamSetupFlag,
         [&] () noexcept
      {
         stringStream.imbue(std::locale{ "" });
      });

      stringStream.str(std::wstring{ });
      stringStream.clear();

      stringStream << number;
      return stringStream.str();
   }
}

struct FileExtensionAndTotalSize
{
    std::wstring fileExtension;
    std::wstring formattedSize;
    std::uintmax_t totalSize;

    FileExtensionAndTotalSize() = default;

    FileExtensionAndTotalSize(
       const std::pair<const std::wstring, std::uintmax_t>& extensionAndSize)
       :
       fileExtension{ extensionAndSize.first },
       totalSize{ extensionAndSize.second }
    {
        const auto fileSize = Controller::ConvertFileSizeToAppropriateUnits(totalSize);
        formattedSize = ConvertToFormattedSizeString(fileSize.first) + L" " + fileSize.second;
    }
};

Q_DECLARE_METATYPE( FileExtensionAndTotalSize );

class ScanBreakdownModel final : public QAbstractTableModel
{
   friend class BreakdownDialog;

   public:

      int rowCount(const QModelIndex& parent) const override;

      int columnCount(const QModelIndex& parent) const override;

      QVariant headerData(
         int section,
         Qt::Orientation orientation,
         int role) const override;

      QVariant data(
         const QModelIndex& index,
         int role) const override;

      void insert(const Tree<VizFile>::Node& node);

   private:

      void FinalizeInsertion();

      std::vector<FileExtensionAndTotalSize> m_fileTypeVector;

      std::unordered_map<std::wstring, std::uintmax_t> m_fileTypeMap;
};

#endif // SCANBREAKDOWNENTRY_H
