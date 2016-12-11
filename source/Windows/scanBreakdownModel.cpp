#include "scanBreakdownModel.h"

#include "../controller.h"

#include <iterator>
#include <mutex>
#include <sstream>
#include <type_traits>

namespace
{
   std::once_flag stringStreamSetupFlag;

   template<
      typename Type,
      typename = std::enable_if<std::is_arithmetic_v<Type>>
   >
   static auto ConvertToFormattedString(const Type& number)
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

int ScanBreakdownModel::rowCount(const QModelIndex& /*parent*/) const
{
   return static_cast<int>(m_fileTypeMap.size());
}

int ScanBreakdownModel::columnCount(const QModelIndex& /*parent*/) const
{
   return 2;
}

QVariant ScanBreakdownModel::headerData(
   int section,
   Qt::Orientation orientation,
   int role) const
{
   if (role != Qt::DisplayRole)
   {
      return { };
   }

   if (orientation == Qt::Orientation::Horizontal)
   {
      switch (section)
      {
         case 0: return QString("File Type");
         case 1: return QString("Cumulative Size");
      }
   }

   return QString::number(section);
}

QVariant ScanBreakdownModel::data(
   const QModelIndex& index,
   int role) const
{
   if (!index.isValid() || role != Qt::DisplayRole)
   {
      return { };
   }

   auto start = std::begin(m_fileTypeMap);
   std::advance(start, index.row());

   const auto fileSize = Controller::ConvertFileSizeToAppropriateUnits(start->second);
   const auto fileSizeString = ConvertToFormattedString(fileSize.first) + L" " + fileSize.second;

   return index.column() == 0
      ? QString::fromStdWString(start->first)
      : QString::fromStdWString(fileSizeString);
}

void ScanBreakdownModel::insert(const TreeNode<VizNode>& node)
{
   const auto& file = node->file;
   if (file.type != FileType::REGULAR)
   {
      return;
   }

   if (file.extension.empty())
   {
      m_fileTypeMap[L"No Extension"] += file.size;
   }
   else
   {
      m_fileTypeMap[file.extension] += file.size;
   }
}
