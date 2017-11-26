#include "scanBreakdownModel.h"

#include "../controller.h"

#include <iterator>

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
         case 0: return QString{ "File Type" };
         case 1: return QString{ "Cumulative Size" };
      }
   }

   return QString::number(section);
}

QVariant ScanBreakdownModel::data(
   const QModelIndex& index,
   int role) const
{
   if (!index.isValid() &&
      role != Qt::UserRole &&
      role != Qt::DisplayRole)
   {
      return { };
   }

   const auto& data = m_fileTypeVector[index.row()];

   if (role == Qt::DisplayRole)
   {
      return index.column() == 0
        ? QString::fromStdWString(data.fileExtension)
        : QString::fromStdWString(data.formattedSize);
   }

   if (role == Qt::UserRole)
   {
      if (index.column() == 0)
      {
         return QString::fromStdWString(data.fileExtension);
      }
      else if (index.column() == 1)
      {
         return static_cast<qulonglong>(data.totalSize);
      }
   }

   return { };
}

void ScanBreakdownModel::insert(const Tree<VizFile>::Node& node)
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

void ScanBreakdownModel::FinalizeInsertion()
{
   assert(!m_fileTypeMap.empty() && m_fileTypeVector.empty());

   std::transform(
      std::begin(m_fileTypeMap),
      std::end(m_fileTypeMap),
      std::back_inserter(m_fileTypeVector),
      [&] (const auto& extensionAndTotalSize)
   {
      const auto& [fileExtension, totalSize] = extensionAndTotalSize;
      const auto& prefix = Constants::FileSize::Prefix::BINARY; //< @todo Pull from preferences...

      const auto [prefixedSize, prefixUnits] =
         Controller::ConvertFileSizeToAppropriateUnits(totalSize, prefix);

      auto sizeLabel =
         Utilities::StringifyWithDigitSeparators(prefixedSize) + L" " + prefixUnits;

      return RowModel
      {
         fileExtension,
         std::move(sizeLabel),
         static_cast<std::uintmax_t>(totalSize)
      };
   });
}
